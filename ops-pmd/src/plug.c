/*
 *  (c) Copyright 2015 Hewlett Packard Enterprise Development LP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License. You may obtain
 *  a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */

/************************************************************************//**
 * @ingroup ops-pmd
 *
 * @file
 * Source file for pluggable module data processing functions.
 ***************************************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include <vswitch-idl.h>
#include <openswitch-idl.h>

#include "pmd.h"
#include "plug.h"
#include "pm_dom.h"

VLOG_DEFINE_THIS_MODULE(plug);

extern struct shash ovs_intfs;
extern YamlConfigHandle global_yaml_handle;

extern int sfpp_sum_verify(unsigned char *);
extern int pm_parse(pm_sfp_serial_id_t *serial_datap, pm_port_t *port);


/*
 * Functions to retieve DOM information
 */
extern void pm_set_a2(pm_port_t *port, pm_sfp_dom_t *a2_data);
extern void set_a2_read_request(pm_port_t *port, pm_sfp_serial_id_t *serial_datap);

/*
  *端口复位
  */
typedef enum
{
  SET_RESET = 0,
  CLEAR_RESET
} clear_reset_t;
static void pm_reset_port(pm_port_t *port);


//
// pm_set_enabled：更改可插拔模块的启用状态
//
// input：none
//
//输出：仅成功
//
int
pm_set_enabled(void)
{
    struct shash_node *node;
    pm_port_t   *port = NULL;

    SHASH_FOR_EACH(node, &ovs_intfs) {
        port = (pm_port_t *)node->data;

        pm_configure_port(port);
    }

    return 0;
}

#define MAX_DEVICE_NAME_LEN 1024

//
// pm_delete_all_data：将所有属性标记为已删除
//除了连接器，它始终存在
//
void
pm_delete_all_data(pm_port_t *port)
{
    DELETE(port, connector_status);
    DELETE_FREE(port, supported_speeds);
    DELETE(port, cable_technology);
    DELETE_FREE(port, cable_length);
    DELETE_FREE(port, max_speed);
    DELETE(port, power_mode);
    DELETE_FREE(port, vendor_name);
    DELETE_FREE(port, vendor_oui);
    DELETE_FREE(port, vendor_part_number);
    DELETE_FREE(port, vendor_revision);
    DELETE_FREE(port, vendor_serial_number);
    DELETE_FREE(port, a0);
    DELETE_FREE(port, a2);
    DELETE_FREE(port, a0_uppers);
}

static bool
pm_get_presence(pm_port_t *port)
{
#ifdef PLATFORM_SIMULATION
    if (NULL != port->module_data) {
        return true;
    }
    return false;
#else
    //存在检测数据
    bool                present;
    uint32_t            result;

    int rc;

    // i2c界面结构
    i2c_bit_op *        reg_op;

    //如果数据无效或操作失败，则重试5次
    int                 retry_count = 2;

    if (0 == strcmp(port->module_device->connector, CONNECTOR_SFP_PLUS)) {
        reg_op = port->module_device->module_signals.sfp.sfpp_mod_present;
    } else if (0 == strcmp(port->module_device->connector,
                           CONNECTOR_QSFP_PLUS)) {
        reg_op = port->module_device->module_signals.qsfp.qsfpp_mod_present;
    } else if (0 == strcmp(port->module_device->connector,
                           CONNECTOR_QSFP28)) {
        reg_op = port->module_device->module_signals.qsfp28.qsfp28p_mod_present;
    } else {
        VLOG_ERR("port is not pluggable: %s", port->instance);
        return false;
    }
retry_read:

    //执行该操作
    rc = i2c_reg_read(global_yaml_handle, port->subsystem, reg_op, &result);

    if (rc != 0) {
        if (retry_count != 0) {
            VLOG_WARN("module presence read failed, retrying: %s",
                      port->instance);
            retry_count--;
            goto retry_read;
        }
        VLOG_ERR("unable to read module presence: %s", port->instance);
        return false;
    }

    //计算出现
    present = (result != 0);

    return present;
#endif
}

static int
pm_read_a0(pm_port_t *port, unsigned char *data, size_t offset)
{
#ifdef PLATFORM_SIMULATION
    memcpy(data, port->module_data, sizeof(pm_sfp_serial_id_t));
    return 0;
#else
    //设备数据
    const YamlDevice *device;

    int                 rc;

    // OPS_TODO：需要读取QSFP模块的准备位（？）

         //获取模块eeprom的设备
    device = yaml_find_device(global_yaml_handle, port->subsystem, port->module_device->module_eeprom);

    rc = i2c_data_read(global_yaml_handle, device, port->subsystem, offset,
                       sizeof(pm_sfp_serial_id_t), data);

    if (rc != 0) {
        VLOG_ERR("module read failed: %s", port->instance);
        return -1;
    }

    return 0;
#endif
}

static int
pm_read_a2(pm_port_t *port, unsigned char *a2_data)
{
#ifdef PLATFORM_SIMULATION
    return -1;
#else
    //设备数据
    const YamlDevice    *device;

    int                 rc;
    char                a2_device_name[MAX_DEVICE_NAME_LEN];

    VLOG_DBG("Read A2 address from yaml files.");
    if (strcmp(port->module_device->connector, CONNECTOR_SFP_PLUS) == 0) {
        strncpy(a2_device_name, port->module_device->module_eeprom, MAX_DEVICE_NAME_LEN);
        strncat(a2_device_name, "_dom", MAX_DEVICE_NAME_LEN);
    }
    else {
        strncpy(a2_device_name, port->module_device->module_eeprom, MAX_DEVICE_NAME_LEN);
    }

    //构建A2设备
    device = yaml_find_device(global_yaml_handle, port->subsystem, a2_device_name);

    rc = i2c_data_read(global_yaml_handle, device, port->subsystem, 0,
                       sizeof(pm_sfp_dom_t), a2_data);

    if (rc != 0) {
        VLOG_ERR("module dom read failed: %s", port->instance);
        return -1;
    }

    return 0;
#endif
}

//
// pm_read_module_state：读取可插拔模块的存在和编号页面
//
// input：port structure
//
//输出：成功0，失败！0
//
// OPS_TODO：这个代码需要被重构以简化和澄清
int
pm_read_module_state(pm_port_t *port)
{
    int             rc;

    //存在检测数据
    bool            present;

    //串行ID数据（SFP +结构）
    pm_sfp_serial_id_t a0;

    // a2页面仅适用于SFP +
    pm_sfp_dom_t a2;
    // unsigned char   a2[PM_SFP_A2_PAGE_SIZE];

      //如果数据无效或操作失败，则重试最多2次
    int             retry_count = 2;
    unsigned char   offset;

    memset(&a0, 0, sizeof(a0));
    memset(&a2, 0, sizeof(a2));

// SFP +和QSFP串行ID数据处于不同的偏移量
         //借此机会获得正确的存在检测操作

    if (0 == strcmp(port->module_device->connector, CONNECTOR_SFP_PLUS)) {
        offset = SFP_SERIAL_ID_OFFSET;
    } else if ((0 == strcmp(port->module_device->connector,
                            CONNECTOR_QSFP_PLUS)) ||
               (0 == strcmp(port->module_device->connector,
                            CONNECTOR_QSFP28))) {
        offset = QSFP_SERIAL_ID_OFFSET;
    } else {
        VLOG_ERR("port is not pluggable: %s", port->instance);
        return -1;
    }

retry_read:
    present = pm_get_presence(port);

    if (!present && false) {    
      //仅当模块以前存在或更新时才更新
               //条目未初始化。
        if ((port->present == true) ||
            (NULL == port->ovs_module_columns.connector)) {
          //从条目中删除当前数据
            port->present = false;
            pm_delete_all_data(port);
            //设置存在枚举
            SET_STATIC_STRING(port, connector, OVSREC_INTERFACE_PM_INFO_CONNECTOR_ABSENT);
            SET_STATIC_STRING(port, connector_status,
                              OVSREC_INTERFACE_PM_INFO_CONNECTOR_STATUS_UNRECOGNIZED);
            VLOG_DBG("module is not present for port: %s", port->instance);
        }
        return 0;    
    }

    if (port->present == false || port->retry == true) {
        //还没有读取A0数据

        VLOG_DBG("module is present for port: %s", port->instance);

        rc = pm_read_a0(port, (unsigned char *)&a0, offset);

        if (rc != 0 && false) {
            if (retry_count != 0) {
                VLOG_DBG("module serial ID data read failed, resetting and retrying: %s",
                         port->instance);
                pm_reset_port(port);
                retry_count--;
                goto retry_read;
            }
            VLOG_WARN("module serial ID data read failed: %s", port->instance);
            pm_delete_all_data(port);
            port->present = true;
            port->retry = true;
            SET_STATIC_STRING(port, connector, OVSREC_INTERFACE_PM_INFO_CONNECTOR_UNKNOWN);
            SET_STATIC_STRING(port, connector_status,
                              OVSREC_INTERFACE_PM_INFO_CONNECTOR_STATUS_UNRECOGNIZED);
            return -1;
        }

        /*
        // do checksum validation
        if (sfpp_sum_verify((unsigned char *)&a0) != 0) {
            if (retry_count != 0) {
                VLOG_DBG("module serial ID data failed checksum, resetting and retrying: %s", port->instance);
                pm_reset_port(port);
                retry_count--;
                goto retry_read;
            }
            VLOG_WARN("module serial ID data failed checksum: %s", port->instance);
            // mark port as present
            port->present = true;
            port->retry = true;
            // delete all attributes, set "unknown" value
            pm_delete_all_data(port);
            SET_STATIC_STRING(port, connector, OVSREC_INTERFACE_PM_INFO_CONNECTOR_UNKNOWN);
            SET_STATIC_STRING(port, connector_status,
                              OVSREC_INTERFACE_PM_INFO_CONNECTOR_STATUS_UNRECOGNIZED);
            return -1;
        }
        */

        // parse the data into important fields, and set it as pending data
        rc = pm_parse(&a0, port);

        if (rc == 0) {
          //将端口标记为现在
            port->present = true;
            port->retry = false;
            set_a2_read_request(port, &a0);
        } else {
            port->retry = true;
            //注意：在失败的情况下，pm_parse已经被记录
                         //一个适当的消息。
            VLOG_DBG("pm_parse has failed for port %s", port->instance);
        }
    }

    if (port->a2_read_requested == false || true) {
        return 0;
    }

retry_read_a2:
    rc = pm_read_a2(port, (unsigned char *)&a2);

    if (rc != 0) {
        if (retry_count != 0) {
            VLOG_DBG("module a2 read failed, retrying: %s", port->instance);
            retry_count--;
            goto retry_read_a2;
        }

        VLOG_WARN("module a2 read failed: %s", port->instance);

        memset(&a2, 0xff, sizeof(a2));
    }

    pm_set_a2(port, &a2);

    port->a2_read_requested = false;

    return 0;
}

//
// pm_read_port_state：读取端口的端口状态
//
// input：port structure
//
//输出：仅成功
//
int
pm_read_port_state(pm_port_t *port)
{
    if (NULL == port) {
        return 0;
    }

    pm_read_module_state(port);

    return 0;
}

//
// pm_read_state：读取所有模块的状态
//
// input：none
//
//输出：无
//
int
pm_read_state(void)
{
    struct shash_node *node;

    SHASH_FOR_EACH(node, &ovs_intfs) {
        pm_port_t *port;

        port = (pm_port_t *)node->data;

        pm_read_port_state(port);
    }

    return 0;
}

//
// pm_configure_qsfp：启用/禁用qsfp模块
//
// input：port structure
//
//输出：无
//
void
pm_configure_qsfp(pm_port_t *port)
{
    uint8_t             data = 0x00;
    unsigned int        idx;

#ifdef PLATFORM_SIMULATION
    if (true == port->split) {
        data = 0x00;
        for (idx = 0; idx < MAX_SPLIT_COUNT; idx++) {

            if (false == port->hw_enable_subport[idx]) {
                data |= (1 << idx);
            }
        }
    } else {
        if (false == port->hw_enable) {
            data = 0x0f;
        } else {
            data = 0x00;
        }
    }

    port->port_enable = data;
    return;
#else
    const YamlDevice    *device;

    int                 rc;

    if (false == port->present) {
        return;
    }

    if (false == port->optical) {
        return;
    }

    // OPS_TODO：需要填写分割指示符
    if (true == port->split) {
        data = 0x00;
        for (idx = 0; idx < MAX_SPLIT_COUNT; idx++) {

            if (false == port->hw_enable_subport[idx]) {
                data |= (1 << idx);
            }
        }
    } else {
        if (false == port->hw_enable) {
            data = 0x0f;
        } else {
            data = 0x00;
        }
    }

    device = yaml_find_device(global_yaml_handle, port->subsystem, port->module_device->module_eeprom);

    rc = i2c_data_write(global_yaml_handle, device, port->subsystem,
                        QSFP_DISABLE_OFFSET, sizeof(data), &data);

    if (0 != rc) {
        VLOG_WARN("Failed to write QSFP enable/disable: %s (%d)",
                  port->instance, rc);
    } else {
        VLOG_DBG("Set QSFP enabled/disable: %s to %0X",
                 port->instance, data);
    }

    return;
#endif
}


//
// pm_reset：重置/清除可插拔模块的复位
//
// input：port structure
//指示清除复位
//
//输出：无
//

static void
pm_reset(pm_port_t *port, clear_reset_t clear)
{
    i2c_bit_op *        reg_op = NULL;
    uint32_t            data;
    int                 rc;

    if (0 == strcmp(port->module_device->connector, CONNECTOR_QSFP_PLUS)) {
        reg_op = port->module_device->module_signals.qsfp.qsfpp_reset;
    } else if (0 == strcmp(port->module_device->connector, CONNECTOR_QSFP28)) {
        reg_op = port->module_device->module_signals.qsfp28.qsfp28p_reset;
    }

    if (NULL == reg_op) {
        VLOG_DBG("port %s does does not have a reset", port->instance);
        return;
    }

    data = clear ? 0 : 0xffu;
    rc = i2c_reg_write(global_yaml_handle, port->subsystem, reg_op, data);

    if (rc != 0) {
        VLOG_WARN("Unable to %s reset for port: %s (%d)",
                  clear ? "clear" : "set", port->instance, rc);
        return;
    }
}

//
// pm_clear_reset：将可插拔模块取出复位
//
// input：port structure
//
//输出：无
//
#define ONE_MILLISECOND 1000000
#define TEN_MILLISECONDS (10*ONE_MILLISECOND)
void
pm_clear_reset(pm_port_t *port)
{
    struct timespec req = {0,TEN_MILLISECONDS};
    pm_reset(port, CLEAR_RESET);
    nanosleep(&req, NULL);
}


//
// pm_reset_port：重置可插拔模块
//
// input：port structure
//
//输出：无
//
static void
pm_reset_port(pm_port_t *port)
{
    struct timespec req = {0,ONE_MILLISECOND};
    pm_reset(port, SET_RESET);
    nanosleep(&req, NULL);
    pm_clear_reset(port);
}

//
// pm_configure_port：启用/禁用可插拔模块
//
// input：port structure
//
//输出：无
//
void
pm_configure_port(pm_port_t *port)
{
#ifdef PLATFORM_SIMULATION
    bool                enabled;

    if ((0 == strcmp(port->module_device->connector, CONNECTOR_QSFP_PLUS)) ||
        (0 == strcmp(port->module_device->connector, CONNECTOR_QSFP28))) {
        pm_configure_qsfp(port);
    } else {
        enabled = port->hw_enable;

        if (enabled) {
            port->port_enable = 1;
        } else {
            port->port_enable = 0;
        }
    }

    return;
#else
    int                 rc;
    uint32_t            data;
    i2c_bit_op          *reg_op;
    bool                enabled;

    if (NULL == port) {
        return;
    }

    if ((0 == strcmp(port->module_device->connector, CONNECTOR_QSFP_PLUS)) ||
        (0 == strcmp(port->module_device->connector, CONNECTOR_QSFP28))) {
        pm_configure_qsfp(port);
        return;
    }

    reg_op = port->module_device->module_signals.sfp.sfpp_tx_disable;

    enabled = port->hw_enable;
    data = enabled ? 0: reg_op->bit_mask;

    rc = i2c_reg_write(global_yaml_handle, port->subsystem, reg_op, data);

    if (rc != 0) {
        VLOG_WARN("Unable to set module disable for port: %s (%d)",
                  port->instance, rc);
        return;
    }

    VLOG_DBG("set port %s to %s",
             port->instance, enabled ? "enabled" : "disabled");
#endif
}

#ifdef PLATFORM_SIMULATION
int
pmd_sim_insert(const char *name, const char *file, struct ds *ds)
{
    struct shash_node *node;
    pm_port_t *port;
    FILE *fp;
    unsigned char *data;

    node = shash_find(&ovs_intfs, name);
    if (NULL == node) {
        ds_put_cstr(ds, "No such interface");
        return -1;
    }
    port = (pm_port_t *)node->data;

    if (NULL != port->module_data) {
        free((void *)port->module_data);
        port->module_data = NULL;
    }

    fp = fopen(file, "r");

    if (NULL == fp) {
        ds_put_cstr(ds, "Can't open file");
        return -1;
    }

    data = (unsigned char *)malloc(sizeof(pm_sfp_serial_id_t));

    if (1 != fread(data, sizeof(pm_sfp_serial_id_t), 1, fp)) {
        ds_put_cstr(ds, "Unable to read data");
        free(data);
        fclose(fp);
        return -1;
    }

    fclose(fp);

    port->module_data = data;

    ds_put_cstr(ds, "Pluggable module inserted");

    return 0;
}

int
pmd_sim_remove(const char *name, struct ds *ds)
{
    struct shash_node *node;
    pm_port_t *port;

    node = shash_find(&ovs_intfs, name);
    if (NULL == node) {
        ds_put_cstr(ds, "No such interface");
        return -1;
    }
    port = (pm_port_t *)node->data;

    if (NULL == port->module_data) {
        ds_put_cstr(ds, "Pluggable module not present");
        return -1;
    }

    free((void *)port->module_data);
    port->module_data = NULL;

    ds_put_cstr(ds, "Pluggable module removed");
    return 0;
}
#endif
