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
 * Source file for pluggable module OVSDB interface functions.
 ***************************************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config-yaml.h"

#include <dynamic-string.h>
#include <vswitch-idl.h>
#include <openswitch-idl.h>

#include "pmd.h"
#include "pm_dom.h"

VLOG_DEFINE_THIS_MODULE(ovsdb_access);

struct ovsdb_idl *idl;

static unsigned int idl_seqno;

#define NAME_IN_DAEMON_TABLE "ops-pmd"

static bool cur_hw_set = false;

struct shash ovs_intfs;
struct shash ovs_subs;

static bool
ovsdb_if_intf_get_hw_enable(const struct ovsrec_interface *intf)
{
    bool hw_enable = false;
    const char *hw_intf_config_enable =
        smap_get(&intf->hw_intf_config, INTERFACE_HW_INTF_CONFIG_MAP_ENABLE);

    if (hw_intf_config_enable) {
        if (!strcmp(INTERFACE_HW_INTF_CONFIG_MAP_ENABLE_TRUE,
                    hw_intf_config_enable)) {
            hw_enable = true;
        }
    }

    return hw_enable;
}

static int
ovsdb_if_intf_create(const struct ovsrec_interface *intf, const char *sub_name)
{
    pm_port_t           *port = NULL;
    const YamlPort      *yaml_port = NULL;
    char                *instance = intf->name;

    //找到此实例的yaml端口
    yaml_port = pm_get_yaml_port(sub_name, instance);

    if (NULL == yaml_port) {
        VLOG_WARN("unable to find YAML configuration for intf instance %s",
                  instance);
        goto end;
    }

    //如果端口不可插拔，那么我们不需要处理它
    if (false == yaml_port->pluggable) {
        VLOG_DBG("port instance %s is not a pluggable module", instance);
        goto end;
    }

    //创建一个新的数据结构来保存端口信息数据
    port = (pm_port_t *)calloc(sizeof(pm_port_t), 1);

    //填写结构
    port->instance = strdup(instance);
    memcpy(&port->uuid, &intf->header_.uuid, sizeof(intf->header_.uuid));
    port->subsystem = strdup(sub_name);

    port->hw_enable = ovsdb_if_intf_get_hw_enable(intf);

    port->module_device = yaml_port;

    //将其标记为缺席，首先，将至少处理一次
    port->present = false;

    port->retry = false;

    //将端口添加到os_intfs窗扇中，以实例为关键字
    shash_add(&ovs_intfs, port->instance, (void *)port);

    VLOG_DBG("pm_port instance (%s) added", instance);

    //应用初始hw_enable状态。
    pm_configure_port(port);

    //清除复位（如果硬件支持复位）
    pm_clear_reset(port);

end:
    return 0;
}

static void
ovsdb_if_intf_modify(const struct ovsrec_interface *intf, pm_port_t *port)
{
    bool hw_enable = false;

    //检查hw_enable列的更改
    hw_enable = ovsdb_if_intf_get_hw_enable(intf);
    if (port->hw_enable != hw_enable) {
        VLOG_DBG("Interface %s hw_enable config changed to %d\n",
                 intf->name, hw_enable);
        port->hw_enable = hw_enable;

        //应用任何端口启用更改
        pm_configure_port(port);
    }
}

static void
ovsdb_if_intf_configure(const struct ovsrec_interface *intf)
{
    struct shash_node *node;
    pm_port_t *port;

    node = shash_find(&ovs_intfs, intf->name);

    if (node != NULL) {
        port = (pm_port_t *)node->data;
        pm_configure_port(port);
    }
}

static void
ovsdb_if_subsys_process(const struct ovsrec_subsystem *ovs_sub)
{
    int i;
    int rc;
    struct shash_node *node;

    node = shash_find(&ovs_subs, ovs_sub->name);
    if (node == NULL) {
        //将子系统添加到窗扇中
        struct uuid *uuid = (struct uuid *) calloc(sizeof(struct uuid), 1);
        memcpy(uuid, &ovs_sub->header_.uuid, sizeof(ovs_sub->header_.uuid));
        shash_add(&ovs_subs, ovs_sub->name, (void *)uuid);

        //读取该子系统所需的YAML系统定义文件
        rc = pm_read_yaml_files(ovs_sub);
        if (rc != 0) {
            VLOG_ERR("Unable to read YAML configuration files for subsystem %s: %d",
                     ovs_sub->name, rc);
            return;
        }
    }

    //确保子系统中存在的所有接口
    //被创建

    for (i = 0; i < ovs_sub->n_interfaces; i++) {
        const struct ovsrec_interface *intf;

        intf = ovs_sub->interfaces[i];
        node = shash_find(&ovs_intfs, intf->name);
        if (node == NULL) {
            ovsdb_if_intf_create(intf, ovs_sub->name);
        }
    }
}

void
pm_ovsdb_update(void)
{
    struct ovsdb_idl_txn *txn;
    const struct ovsrec_interface *intf;
    const struct ovsrec_daemon *db_daemon;
    pm_port_t   *port = NULL;
    struct shash_node *node;

    txn = ovsdb_idl_txn_create(idl);

    //循环遍历所有接口并更新可插拔模块
         //如果有必要，在数据库中的信息。
    SHASH_FOR_EACH(node, &ovs_intfs) {
        struct ovs_module_info *module;
        struct ovs_module_dom_info *module_dom;
        struct smap pm_info;

        port = (pm_port_t *)node->data;

        //如果没有端口，它可能不可插拔
        if (NULL == port) {
            continue;
        }

        intf = ovsrec_interface_get_for_uuid(idl, &port->uuid);
        if (NULL == intf) {
            VLOG_ERR("No DB entry found for hw interface %s\n",
                     port->instance);
            continue;
        }
        if (false == port->module_info_changed) {
            continue;
        }

        module = &port->ovs_module_columns;
        //设置pm_info地图
        smap_init(&pm_info);
        /*
        if (module->cable_length) {
            ;//smap_add(&pm_info, "cable_length", module->cable_length);
        }
        if (module->cable_technology) {
            smap_add(&pm_info, "cable_technology", module->cable_technology);
        }*/
        if (module->connector) {
            smap_add(&pm_info, "connector", module->connector);
        }
        if (module->connector_status) {        	
            smap_add(&pm_info, "connector_status", module->connector_status);
        }
        if (module->supported_speeds) {
            smap_add(&pm_info, "supported_speeds", module->supported_speeds);
        }
        //if(cnt > 50)
        if (module->max_speed) {
            smap_add(&pm_info, "max_speed", module->max_speed);
        }/*
        if (module->power_mode) {
            smap_add(&pm_info, "power_mode", module->power_mode);
        }
        if (module->vendor_name) {
            smap_add(&pm_info, "vendor_name", module->vendor_name);
        }*/
        /*if (module->vendor_oui) {
            smap_add(&pm_info, "vendor_oui", module->vendor_oui);
        }*//*
        if (module->vendor_part_number) {
            smap_add(&pm_info, "vendor_part_number",
                     module->vendor_part_number);
        }
        if (module->vendor_revision) {
            smap_add(&pm_info, "vendor_revision", module->vendor_revision);
        }
        if (module->vendor_serial_number) {
            smap_add(&pm_info, "vendor_serial_number",
                     module->vendor_serial_number);
        }*/

				
        // Update diagnostics key values
        module_dom = &port->ovs_module_dom_columns;
/*
        if (module_dom->temperature) {
            smap_add(&pm_info, "temperature", module_dom->temperature);
        }
        if (module_dom->temperature_high_alarm) {
            smap_add(&pm_info, "temperature_high_alarm", module_dom->temperature_high_alarm);
        }
        if (module_dom->temperature_low_alarm) {
            smap_add(&pm_info, "temperature_low_alarm", module_dom->temperature_low_alarm);
        }
        if (module_dom->temperature_high_warning) {
            smap_add(&pm_info, "temperature_high_warning", module_dom->temperature_high_warning);
        }
        if (module_dom->temperature_low_warning) {
            smap_add(&pm_info, "temperature_low_warning", module_dom->temperature_low_warning);
        }
        if (module_dom->temperature_high_alarm_threshold) {
            smap_add(&pm_info, "temperature_high_alarm_threshold", module_dom->temperature_high_alarm_threshold);
        }
        if (module_dom->temperature_low_alarm_threshold) {
            smap_add(&pm_info, "temperature_low_alarm_threshold", module_dom->temperature_low_alarm_threshold);
        }
        if (module_dom->temperature_high_warning_threshold) {
            smap_add(&pm_info, "temperature_high_warning_threshold", module_dom->temperature_high_warning_threshold);
        }
        if (module_dom->temperature_low_warning_threshold) {
            smap_add(&pm_info, "temperature_low_warning_threshold", module_dom->temperature_low_warning_threshold);
        }

        if (module_dom->vcc) {
            smap_add(&pm_info, "vcc", module_dom->vcc);
        }
        if (module_dom->vcc_high_alarm) {
            smap_add(&pm_info, "vcc_high_alarm", module_dom->vcc_high_alarm);
        }
        if (module_dom->vcc_low_alarm) {
            smap_add(&pm_info, "vcc_low_alarm", module_dom->vcc_low_alarm);
        }
        if (module_dom->vcc_high_warning) {
            smap_add(&pm_info, "vcc_high_warning", module_dom->vcc_high_warning);
        }
        if (module_dom->vcc_low_warning) {
            smap_add(&pm_info, "vcc_low_warning", module_dom->vcc_low_warning);
        }
        if (module_dom->vcc_high_alarm_threshold) {
            smap_add(&pm_info, "vcc_high_alarm_threshold", module_dom->vcc_high_alarm_threshold);
        }
        if (module_dom->vcc_low_alarm_threshold) {
            smap_add(&pm_info, "vcc_low_alarm_threshold", module_dom->vcc_low_alarm_threshold);
        }
        if (module_dom->vcc_high_warning_threshold) {
            smap_add(&pm_info, "vcc_high_warning_threshold", module_dom->vcc_high_warning_threshold);
        }
        if (module_dom->vcc_low_warning_threshold) {
            smap_add(&pm_info, "vcc_low_warning_threshold", module_dom->vcc_low_warning_threshold);
        }

        if (module_dom->tx_bias) {
            smap_add(&pm_info, "tx_bias", module_dom->tx_bias);
        }
        if (module_dom->tx_bias_high_alarm) {
            smap_add(&pm_info, "tx_bias_high_alarm", module_dom->tx_bias_high_alarm);
        }
        if (module_dom->tx_bias_low_alarm) {
            smap_add(&pm_info, "tx_bias_low_alarm", module_dom->tx_bias_low_alarm);
        }
        if (module_dom->tx_bias_high_warning) {
            smap_add(&pm_info, "tx_bias_high_warning", module_dom->tx_bias_high_warning);
        }
        if (module_dom->tx_bias_low_warning) {
            smap_add(&pm_info, "tx_bias_low_warning", module_dom->tx_bias_low_warning);
        }
        if (module_dom->tx_bias_high_alarm_threshold) {
            smap_add(&pm_info, "tx_bias_high_alarm_threshold", module_dom->tx_bias_high_alarm_threshold);
        }
        if (module_dom->tx_bias_low_alarm_threshold) {
            smap_add(&pm_info, "tx_bias_low_alarm_threshold", module_dom->tx_bias_low_alarm_threshold);
        }
        if (module_dom->tx_bias_high_warning_threshold) {
            smap_add(&pm_info, "tx_bias_high_warning_threshold", module_dom->tx_bias_high_warning_threshold);
        }
        if (module_dom->tx_bias_low_warning_threshold) {
            smap_add(&pm_info, "tx_bias_low_warning_threshold", module_dom->tx_bias_low_warning_threshold);
        }

        if (module_dom->rx_power) {
            smap_add(&pm_info, "rx_power", module_dom->rx_power);
        }
        if (module_dom->rx_power_high_alarm) {
            smap_add(&pm_info, "rx_power_high_alarm", module_dom->rx_power_high_alarm);
        }
        if (module_dom->rx_power_low_alarm) {
            smap_add(&pm_info, "rx_power_low_alarm", module_dom->rx_power_low_alarm);
        }
        if (module_dom->rx_power_high_warning) {
            smap_add(&pm_info, "rx_power_high_warning", module_dom->rx_power_high_warning);
        }
        if (module_dom->rx_power_low_warning) {
            smap_add(&pm_info, "rx_power_low_warning", module_dom->rx_power_low_warning);
        }
        if (module_dom->rx_power_high_alarm_threshold) {
            smap_add(&pm_info, "rx_power_high_alarm_threshold", module_dom->rx_power_high_alarm_threshold);
        }
        if (module_dom->rx_power_low_alarm_threshold) {
            smap_add(&pm_info, "rx_power_low_alarm_threshold", module_dom->rx_power_low_alarm_threshold);
        }
        if (module_dom->rx_power_high_warning_threshold) {
            smap_add(&pm_info, "rx_power_high_warning_threshold", module_dom->rx_power_high_warning_threshold);
        }
        if (module_dom->rx_power_low_warning_threshold) {
            smap_add(&pm_info, "rx_power_low_warning_threshold", module_dom->rx_power_low_warning_threshold);
        }

        if (module_dom->tx_power) {
            smap_add(&pm_info, "tx_power", module_dom->tx_power);
        }
        if (module_dom->rx_power_high_alarm) {
            smap_add(&pm_info, "tx_power_high_alarm", module_dom->tx_power_high_alarm);
        }
        if (module_dom->tx_power_low_alarm) {
            smap_add(&pm_info, "tx_power_low_alarm", module_dom->tx_power_low_alarm);
        }
        if (module_dom->tx_power_high_warning) {
            smap_add(&pm_info, "tx_power_high_warning", module_dom->tx_power_high_warning);
        }
        if (module_dom->tx_power_low_warning) {
            smap_add(&pm_info, "tx_power_low_warning", module_dom->tx_power_low_warning);
        }
        if (module_dom->tx_power_high_alarm_threshold) {
            smap_add(&pm_info, "tx_power_high_alarm_threshold", module_dom->tx_power_high_alarm_threshold);
        }
        if (module_dom->tx_power_low_alarm_threshold) {
            smap_add(&pm_info, "tx_power_low_alarm_threshold", module_dom->tx_power_low_alarm_threshold);
        }
        if (module_dom->tx_power_high_warning_threshold) {
            smap_add(&pm_info, "tx_power_high_warning_threshold", module_dom->tx_power_high_warning_threshold);
        }
        if (module_dom->tx_power_low_warning_threshold) {
            smap_add(&pm_info, "tx_power_low_warning_threshold", module_dom->tx_power_low_warning_threshold);
        }



        if (module_dom->tx1_bias) {
            smap_add(&pm_info, "tx1_bias", module_dom->tx1_bias);
        }
        if (module_dom->rx1_power) {
            smap_add(&pm_info, "rx1_power", module_dom->rx1_power);
        }

        if (module_dom->tx1_bias_high_alarm) {
            smap_add(&pm_info, "tx1_bias_high_alarm", module_dom->tx1_bias_high_alarm);
        }
        if (module_dom->tx1_bias_low_alarm) {
            smap_add(&pm_info, "tx1_bias_low_alarm", module_dom->tx1_bias_low_alarm);
        }
        if (module_dom->tx1_bias_high_warning) {
            smap_add(&pm_info, "tx1_bias_high_warning", module_dom->tx1_bias_high_warning);
        }
        if (module_dom->tx1_bias_low_warning) {
            smap_add(&pm_info, "tx1_bias_low_warning", module_dom->tx1_bias_low_warning);
        }
        if (module_dom->tx1_bias_high_alarm_threshold) {
            smap_add(&pm_info, "tx1_bias_high_alarm_threshold", module_dom->tx1_bias_high_alarm_threshold);
        }
        if (module_dom->tx1_bias_low_alarm_threshold) {
            smap_add(&pm_info, "tx1_bias_low_alarm_threshold", module_dom->tx1_bias_low_alarm_threshold);
        }
        if (module_dom->tx1_bias_high_warning_threshold) {
            smap_add(&pm_info, "tx1_bias_high_warning_threshold", module_dom->tx1_bias_high_warning_threshold);
        }
        if (module_dom->tx1_bias_low_warning_threshold) {
            smap_add(&pm_info, "tx1_bias_low_warning_threshold", module_dom->tx1_bias_low_warning_threshold);
        }

        if (module_dom->rx1_power_high_alarm) {
            smap_add(&pm_info, "rx1_power_high_alarm", module_dom->rx1_power_high_alarm);
        }
        if (module_dom->rx1_power_low_alarm) {
            smap_add(&pm_info, "rx1_power_low_alarm", module_dom->rx1_power_low_alarm);
        }
        if (module_dom->rx1_power_high_warning) {
            smap_add(&pm_info, "rx1_power_high_warning", module_dom->rx1_power_high_warning);
        }
        if (module_dom->rx1_power_low_warning) {
            smap_add(&pm_info, "rx1_power_low_warning", module_dom->rx1_power_low_warning);
        }
        if (module_dom->rx1_power_high_alarm_threshold) {
            smap_add(&pm_info, "rx1_power_high_alarm_threshold", module_dom->rx1_power_high_alarm_threshold);
        }
        if (module_dom->rx1_power_low_alarm_threshold) {
            smap_add(&pm_info, "rx1_power_low_alarm_threshold", module_dom->rx1_power_low_alarm_threshold);
        }
        if (module_dom->rx1_power_high_warning_threshold) {
            smap_add(&pm_info, "rx1_power_high_warning_threshold", module_dom->rx1_power_high_warning_threshold);
        }
        if (module_dom->rx1_power_low_warning_threshold) {
            smap_add(&pm_info, "rx1_power_low_warning_threshold", module_dom->rx1_power_low_warning_threshold);
        }



        if (module_dom->tx2_bias) {
            smap_add(&pm_info, "tx2_bias", module_dom->tx2_bias);
        }
        if (module_dom->rx2_power) {
            smap_add(&pm_info, "rx2_power", module_dom->rx2_power);
        }

        if (module_dom->tx2_bias_high_alarm) {
            smap_add(&pm_info, "tx2_bias_high_alarm", module_dom->tx2_bias_high_alarm);
        }
        if (module_dom->tx2_bias_low_alarm) {
            smap_add(&pm_info, "tx2_bias_low_alarm", module_dom->tx2_bias_low_alarm);
        }
        if (module_dom->tx2_bias_high_warning) {
            smap_add(&pm_info, "tx2_bias_high_warning", module_dom->tx2_bias_high_warning);
        }
        if (module_dom->tx2_bias_low_warning) {
            smap_add(&pm_info, "tx2_bias_low_warning", module_dom->tx2_bias_low_warning);
        }
        if (module_dom->tx2_bias_high_alarm_threshold) {
            smap_add(&pm_info, "tx2_bias_high_alarm_threshold", module_dom->tx2_bias_high_alarm_threshold);
        }
        if (module_dom->tx2_bias_low_alarm_threshold) {
            smap_add(&pm_info, "tx2_bias_low_alarm_threshold", module_dom->tx2_bias_low_alarm_threshold);
        }
        if (module_dom->tx2_bias_high_warning_threshold) {
            smap_add(&pm_info, "tx2_bias_high_warning_threshold", module_dom->tx2_bias_high_warning_threshold);
        }
        if (module_dom->tx2_bias_low_warning_threshold) {
            smap_add(&pm_info, "tx2_bias_low_warning_threshold", module_dom->tx2_bias_low_warning_threshold);
        }

        if (module_dom->rx2_power_high_alarm) {
            smap_add(&pm_info, "rx2_power_high_alarm", module_dom->rx2_power_high_alarm);
        }
        if (module_dom->rx2_power_low_alarm) {
            smap_add(&pm_info, "rx2_power_low_alarm", module_dom->rx2_power_low_alarm);
        }
        if (module_dom->rx2_power_high_warning) {
            smap_add(&pm_info, "rx2_power_high_warning", module_dom->rx2_power_high_warning);
        }
        if (module_dom->rx2_power_low_warning) {
            smap_add(&pm_info, "rx2_power_low_warning", module_dom->rx2_power_low_warning);
        }
        if (module_dom->rx2_power_high_alarm_threshold) {
            smap_add(&pm_info, "rx2_power_high_alarm_threshold", module_dom->rx2_power_high_alarm_threshold);
        }
        if (module_dom->rx2_power_low_alarm_threshold) {
            smap_add(&pm_info, "rx2_power_low_alarm_threshold", module_dom->rx2_power_low_alarm_threshold);
        }
        if (module_dom->rx2_power_high_warning_threshold) {
            smap_add(&pm_info, "rx2_power_high_warning_threshold", module_dom->rx2_power_high_warning_threshold);
        }
        if (module_dom->rx2_power_low_warning_threshold) {
            smap_add(&pm_info, "rx2_power_low_warning_threshold", module_dom->rx2_power_low_warning_threshold);
        }



        if (module_dom->tx3_bias) {
            smap_add(&pm_info, "tx3_bias", module_dom->tx3_bias);
        }
        if (module_dom->rx3_power) {
            smap_add(&pm_info, "rx3_power", module_dom->rx3_power);
        }

        if (module_dom->tx3_bias_high_alarm) {
            smap_add(&pm_info, "tx3_bias_high_alarm", module_dom->tx3_bias_high_alarm);
        }
        if (module_dom->tx3_bias_low_alarm) {
            smap_add(&pm_info, "tx3_bias_low_alarm", module_dom->tx3_bias_low_alarm);
        }
        if (module_dom->tx3_bias_high_warning) {
            smap_add(&pm_info, "tx3_bias_high_warning", module_dom->tx3_bias_high_warning);
        }
        if (module_dom->tx3_bias_low_warning) {
            smap_add(&pm_info, "tx3_bias_low_warning", module_dom->tx3_bias_low_warning);
        }
        if (module_dom->tx3_bias_high_alarm_threshold) {
            smap_add(&pm_info, "tx3_bias_high_alarm_threshold", module_dom->tx3_bias_high_alarm_threshold);
        }
        if (module_dom->tx3_bias_low_alarm_threshold) {
            smap_add(&pm_info, "tx3_bias_low_alarm_threshold", module_dom->tx3_bias_low_alarm_threshold);
        }
        if (module_dom->tx3_bias_high_warning_threshold) {
            smap_add(&pm_info, "tx3_bias_high_warning_threshold", module_dom->tx3_bias_high_warning_threshold);
        }
        if (module_dom->tx3_bias_low_warning_threshold) {
            smap_add(&pm_info, "tx3_bias_low_warning_threshold", module_dom->tx3_bias_low_warning_threshold);
        }

        if (module_dom->rx3_power_high_alarm) {
            smap_add(&pm_info, "rx3_power_high_alarm", module_dom->rx3_power_high_alarm);
        }
        if (module_dom->rx3_power_low_alarm) {
            smap_add(&pm_info, "rx3_power_low_alarm", module_dom->rx3_power_low_alarm);
        }
        if (module_dom->rx3_power_high_warning) {
            smap_add(&pm_info, "rx3_power_high_warning", module_dom->rx3_power_high_warning);
        }
        if (module_dom->rx3_power_low_warning) {
            smap_add(&pm_info, "rx3_power_low_warning", module_dom->rx3_power_low_warning);
        }
        if (module_dom->rx3_power_high_alarm_threshold) {
            smap_add(&pm_info, "rx3_power_high_alarm_threshold", module_dom->rx3_power_high_alarm_threshold);
        }
        if (module_dom->rx3_power_low_alarm_threshold) {
            smap_add(&pm_info, "rx3_power_low_alarm_threshold", module_dom->rx3_power_low_alarm_threshold);
        }
        if (module_dom->rx3_power_high_warning_threshold) {
            smap_add(&pm_info, "rx3_power_high_warning_threshold", module_dom->rx3_power_high_warning_threshold);
        }
        if (module_dom->rx3_power_low_warning_threshold) {
            smap_add(&pm_info, "rx3_power_low_warning_threshold", module_dom->rx3_power_low_warning_threshold);
        }



        if (module_dom->tx4_bias) {
            smap_add(&pm_info, "tx4_bias", module_dom->tx4_bias);
        }
        if (module_dom->rx4_power) {
            smap_add(&pm_info, "rx4_power", module_dom->rx4_power);
        }

        if (module_dom->tx4_bias_high_alarm) {
            smap_add(&pm_info, "tx4_bias_high_alarm", module_dom->tx4_bias_high_alarm);
        }
        if (module_dom->tx4_bias_low_alarm) {
            smap_add(&pm_info, "tx4_bias_low_alarm", module_dom->tx4_bias_low_alarm);
        }
        if (module_dom->tx4_bias_high_warning) {
            smap_add(&pm_info, "tx4_bias_high_warning", module_dom->tx4_bias_high_warning);
        }
        if (module_dom->tx4_bias_low_warning) {
            smap_add(&pm_info, "tx4_bias_low_warning", module_dom->tx4_bias_low_warning);
        }
        if (module_dom->tx4_bias_high_alarm_threshold) {
            smap_add(&pm_info, "tx4_bias_high_alarm_threshold", module_dom->tx4_bias_high_alarm_threshold);
        }
        if (module_dom->tx4_bias_low_alarm_threshold) {
            smap_add(&pm_info, "tx4_bias_low_alarm_threshold", module_dom->tx4_bias_low_alarm_threshold);
        }
        if (module_dom->tx4_bias_high_warning_threshold) {
            smap_add(&pm_info, "tx4_bias_high_warning_threshold", module_dom->tx4_bias_high_warning_threshold);
        }
        if (module_dom->tx4_bias_low_warning_threshold) {
            smap_add(&pm_info, "tx4_bias_low_warning_threshold", module_dom->tx4_bias_low_warning_threshold);
        }

        if (module_dom->rx4_power_high_alarm) {
            smap_add(&pm_info, "rx4_power_high_alarm", module_dom->rx4_power_high_alarm);
        }
        if (module_dom->rx4_power_low_alarm) {
            smap_add(&pm_info, "rx4_power_low_alarm", module_dom->rx4_power_low_alarm);
        }
        if (module_dom->rx4_power_high_warning) {
            smap_add(&pm_info, "rx4_power_high_warning", module_dom->rx4_power_high_warning);
        }
        if (module_dom->rx4_power_low_warning) {
            smap_add(&pm_info, "rx4_power_low_warning", module_dom->rx4_power_low_warning);
        }
        if (module_dom->rx4_power_high_alarm_threshold) {
            smap_add(&pm_info, "rx4_power_high_alarm_threshold", module_dom->rx4_power_high_alarm_threshold);
        }
        if (module_dom->rx4_power_low_alarm_threshold) {
            smap_add(&pm_info, "rx4_power_low_alarm_threshold", module_dom->rx4_power_low_alarm_threshold);
        }
        if (module_dom->rx4_power_high_warning_threshold) {
            smap_add(&pm_info, "rx4_power_high_warning_threshold", module_dom->rx4_power_high_warning_threshold);
        }*/
        if (module_dom) {
            ;//smap_add(&pm_info, "rx4_power_low_warning_threshold", module_dom->rx4_power_low_warning_threshold);
        }

        ovsrec_interface_set_pm_info(intf, &pm_info);
        smap_destroy(&pm_info);

        //清除端口的模块信息更新状态
        port->module_info_changed = false;
    }
          
    if (!cur_hw_set) {
        OVSREC_DAEMON_FOR_EACH(db_daemon, idl) {
        	  if (strcmp(db_daemon->name, NAME_IN_DAEMON_TABLE) == 0) {
                ovsrec_daemon_set_cur_hw(db_daemon, (int64_t) 1);
                VLOG_WARN("%s(%d)started+++++++db_daemon->cur_hw=%d", __FUNCTION__, __LINE__, (int)db_daemon->cur_hw);
                cur_hw_set = true;
                break;
            }
        }
    }

    ovsdb_idl_txn_commit_block(txn);
    ovsdb_idl_txn_destroy(txn);

}

static void
pmd_free_pm_port(pm_port_t *port)
{
    pm_delete_all_data(port);
    free(port->instance);
    free(port);
}

static int
pm_daemon_subscribe(void)
{
    ovsdb_idl_add_table(idl, &ovsrec_table_daemon);
    ovsdb_idl_add_column(idl, &ovsrec_daemon_col_name);
    ovsdb_idl_add_column(idl, &ovsrec_daemon_col_cur_hw);
    ovsdb_idl_omit_alert(idl, &ovsrec_daemon_col_cur_hw);

    return 0;
}

static int
pm_intf_subscribe(void)
{
  //初始化端口数据哈希
    shash_init(&ovs_intfs);

    ovsdb_idl_add_table(idl, &ovsrec_table_interface);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_name);
    ovsdb_idl_add_column(idl, &ovsrec_interface_col_pm_info);
    ovsdb_idl_omit_alert(idl, &ovsrec_interface_col_pm_info);

    ovsdb_idl_add_column(idl, &ovsrec_interface_col_hw_intf_config);

    return 0;
}

static int
pm_subsystem_subscribe(void)
{
    shash_init(&ovs_subs);

    ovsdb_idl_add_table(idl, &ovsrec_table_subsystem);
    ovsdb_idl_add_column(idl, &ovsrec_subsystem_col_name);
    ovsdb_idl_add_column(idl, &ovsrec_subsystem_col_hw_desc_dir);
    ovsdb_idl_add_column(idl, &ovsrec_subsystem_col_interfaces);

    return 0;
}

static void
pmd_configure(struct ovsdb_idl *idl)
{
    const struct ovsrec_subsystem *subsys;
    const struct ovsrec_interface *intf;

    //启动时读取进程子系统条目。
    OVSREC_SUBSYSTEM_FOR_EACH(subsys, idl) {
        VLOG_DBG("Adding Subsystem %s to pmd data store\n", subsys->name);
        ovsdb_if_subsys_process(subsys);
    }

    //启动时处理接口条目。
    OVSREC_INTERFACE_FOR_EACH(intf, idl) {
        VLOG_DBG("Adding Interface %s to pmd data store\n", intf->name);
        ovsdb_if_intf_configure(intf);
    }

}

void
pmd_reconfigure(struct ovsdb_idl *idl)
{
    const struct ovsrec_subsystem *subsys;
    const struct ovsrec_interface *intf;
    unsigned int new_idl_seqno = ovsdb_idl_get_seqno(idl);
    pm_port_t *port;
    struct shash_node *node;
    struct shash_node *next;

    if (new_idl_seqno == idl_seqno){
        return;
    }

    idl_seqno = new_idl_seqno;

    //处理已删除的界面
    SHASH_FOR_EACH_SAFE(node, next, &ovs_intfs) {
        const struct ovsrec_interface *tmp_if;
        port = (pm_port_t *) node->data;

        tmp_if = ovsrec_interface_get_for_uuid(idl, &port->uuid);
        if (NULL == tmp_if) {
            struct shash_node *delete_node;
            VLOG_DBG("Deleted Interface %s\n", port->instance);
            delete_node = shash_find(&ovs_intfs, port->instance);
            port = (pm_port_t *) delete_node->data;
            shash_delete(&ovs_intfs, delete_node);
            pmd_free_pm_port(port);
        }
    }

    ///处理已删除的子系统
    SHASH_FOR_EACH_SAFE(node, next, &ovs_subs) {
        const struct ovsrec_subsystem *tmp_sub;
        struct uuid *uuid;

        uuid = (struct uuid *) node->data;

        tmp_sub = ovsrec_subsystem_get_for_uuid(idl, uuid);
        if (NULL == tmp_sub) {
            struct shash_node *delete_node;
            delete_node = shash_find(&ovs_subs, node->name);
            VLOG_DBG("Deleted subsystem %s\n", node->name);
            shash_delete(&ovs_subs, delete_node);
            free(uuid);
            // OPS_TODO：删除配置子系统
        }
    }

    //处理添加/删除的子系统。
    OVSREC_SUBSYSTEM_FOR_EACH(subsys, idl) {
        ovsdb_if_subsys_process(subsys);
    }

    //处理修改后的界面。
    OVSREC_INTERFACE_FOR_EACH(intf, idl) {
        node = shash_find(&ovs_intfs, intf->name);
        if (NULL != node) {
            port = (pm_port_t *) node->data;
            //处理修改后的界面。
            ovsdb_if_intf_modify(intf, port);
        }
    }
}

int
pm_ovsdb_if_init(const char *remote)
{
    idl = ovsdb_idl_create(remote, &ovsrec_idl_class, false, true);

    idl_seqno = ovsdb_idl_get_seqno(idl);
    ovsdb_idl_set_lock(idl, "ops_pmd");
    ovsdb_idl_verify_write_only(idl);

    //订阅子系统表。
    pm_subsystem_subscribe();

    //订阅界面表。
    pm_intf_subscribe();

    //订阅守护进程表。
    pm_daemon_subscribe();

    //处理初始配置
    pmd_configure(idl);

    return 0;
}

/**********************************************************************/
/*                               DEBUG                                */
/**********************************************************************/
static void
pm_interface_dump(struct ds *ds, pm_port_t *port)
{
    struct ovs_module_info *module;

    module = &port->ovs_module_columns;
    ds_put_format(ds, "Pluggable info for Interface %s:\n", port->instance);
    if (module->cable_length) {
        ds_put_format(ds, "    cable_length           = %s\n",
                      module->cable_length);
    }
    if (module->cable_technology) {
        ds_put_format(ds, "    cable_technology       = %s\n",
                      module->cable_technology);
    }
    if (module->connector) {
        ds_put_format(ds, "    connector              = %s\n",
                      module->connector);
    }
    if (module->connector_status) {
        ds_put_format(ds, "    connector_status       = %s\n",
                      module->connector_status);
    }
    if (module->supported_speeds) {
        ds_put_format(ds, "    supported_speeds       = %s\n",
                      module->supported_speeds);
    }
    if (module->max_speed) {
        ds_put_format(ds, "    max_speed              = %s\n",
                      module->max_speed);
    }
    if (module->power_mode) {
        ds_put_format(ds, "    power_mode             = %s\n",
                      module->power_mode);
    }
    if (module->vendor_name) {
        ds_put_format(ds, "    vendor_name            = %s\n",
                      module->vendor_name);
    }
    if (module->vendor_oui) {
        ds_put_format(ds, "    vendor_oui             = %s\n",
                      module->vendor_oui);
    }
    if (module->vendor_part_number) {
        ds_put_format(ds, "    vendor_part_number     = %s\n",
                      module->vendor_part_number);
    }
    if (module->vendor_revision) {
        ds_put_format(ds, "    vendor_revision        = %s\n",
                      module->vendor_revision);
    }
    if (module->vendor_serial_number) {
        ds_put_format(ds, "    vendor_serial_number   = %s\n",
                      module->vendor_serial_number);
    }
}

static void
pm_interfaces_dump(struct ds *ds, int argc, const char *argv[])
{
    struct shash_node *sh_node;
    pm_port_t *port = NULL;

    if (argc > 2) {
        sh_node = shash_find(&ovs_intfs, argv[2]);
        if (NULL != sh_node) {
            port = (pm_port_t *)sh_node->data;
            if (port){
                pm_interface_dump(ds, port);
            }
        }
    } else {
        ds_put_cstr(ds, "================ Interfaces ================\n");

        SHASH_FOR_EACH(sh_node, &ovs_intfs) {
            port = (pm_port_t *)sh_node->data;
            if (port){
                pm_interface_dump(ds, port);
            }
        }
    }
}

/**
  * @细节
  *转储整个守护程序或指定的各个组件的调试数据
  *在命令行。
  */
void
pm_debug_dump(struct ds *ds, int argc, const char *argv[])
{
    const char *table_name = NULL;

    if (argc > 1) {
        table_name = argv[1];

        if (!strcmp(table_name, "interface")) {
            pm_interfaces_dump(ds, argc, argv);
        }
    } else {
        pm_interfaces_dump(ds, 0, NULL);
    }
}
