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
 * Source file for pluggable module config-yaml interface functions.
 ***************************************************************************/

#include <string.h>

#include <vswitch-idl.h>
#include <openswitch-idl.h>

#include "config-yaml.h"
#include "pmd.h"

#define YAML_DEVICES    "devices.yaml"
#define YAML_PORTS      "ports.yaml"

VLOG_DEFINE_THIS_MODULE(config);

YamlConfigHandle global_yaml_handle;
extern struct shash ovs_intfs;


void
pm_config_init(void)
{
    global_yaml_handle = yaml_new_config_handle();
}
/*
  * pm_get_yaml_port：通过实例名称找到匹配的端口
  *
  * input：子系统名称，实例字符串
  *
  *输出：指向匹配YamlPort对象的指针
  */
const YamlPort *
pm_get_yaml_port(const char *subsystem, const char *instance)
{
    size_t          count;
    size_t          idx;
    const YamlPort *yaml_port;

    count = yaml_get_port_count(global_yaml_handle, subsystem);

    //查询YAML数据中的端口
    for (idx = 0; idx < count; idx++) {
        yaml_port = yaml_get_port(global_yaml_handle, subsystem, idx);

        //找到匹配的实例
        if (strcmp(yaml_port->name, instance) == 0) {
            return(yaml_port);
        }
    }

    return(NULL);
}

/*
  * pm_create_a2_devices：为sfpp模块创建隐含的a2设备
  *
  *输入：无
  *
  *输出：无
  */
void
pm_create_a2_devices(void)
{
    const YamlPort      *yaml_port;
    const YamlDevice    *a0_device;
    YamlDevice          a2_device;
    char                *new_name;
    int                 name_len;
    int                 rc;
    struct shash_node   *node;

    SHASH_FOR_EACH(node, &ovs_intfs) {
        pm_port_t *port;

        port = (pm_port_t *)node->data;

        yaml_port = pm_get_yaml_port(port->subsystem, port->instance);

        //如果端口不可插拔，请跳过它
        if (false == yaml_port->pluggable) {
            continue;
        }

        //如果端口不是SFPP，请跳过它
        if (strcmp(yaml_port->connector, SFPP) != 0) {
            continue;
        }

        //为端口找到匹配的a0设备
        a0_device = yaml_find_device(global_yaml_handle, port->subsystem, yaml_port->module_eeprom);

        if (NULL == a0_device) {
            VLOG_WARN("Unable to find eeprom device for SFP+ port: %s",
                      yaml_port->name);
            continue;
        }

        //构建一个新的YamlDevice

        //分配新名称
        name_len = strlen(a0_device->name) + 5;

        //创建A2名称作为附加“_A2”的旧设备名称
        new_name = (char *)malloc(name_len);
        strncpy(new_name, a0_device->name, name_len);
        strncat(new_name, "_dom", name_len);

        //填写设备数据，这主要与a0设备匹配
        a2_device.name = new_name;
        a2_device.bus = a0_device->bus;
        a2_device.dev_type = a0_device->dev_type;
        a2_device.address = PM_SFP_A2_I2C_ADDRESS;
        a2_device.pre = a0_device->pre;
        a2_device.post = a0_device->post;

        //将新设备条目添加到yaml数据中
        rc = yaml_add_device(global_yaml_handle, port->subsystem, new_name, &a2_device);

        if (0 != rc) {
            VLOG_ERR("Unable to add A2 device for SFP+ port: %s",
                     yaml_port->name);
            //继续执行，A2数据将不可用于端口
        }

        free(new_name);
    }
}

/*
  * pm_read_yaml_files：读取可插拔模块的相关系统文件
  *
  * input：ovsrec_subsystem
  *
  *输出：0成功，非零失败
  */
int
pm_read_yaml_files(const struct ovsrec_subsystem *subsys)
{
    int rc;

    rc = yaml_add_subsystem(global_yaml_handle, subsys->name, subsys->hw_desc_dir);

    if (0 != rc) {
        VLOG_ERR("yaml_add_subsystem failed");
        goto end;
    }

    //读取设备
    rc = yaml_parse_devices(global_yaml_handle, subsys->name);

    if (0 != rc) {
        VLOG_ERR("yaml_parse_devices failed: %s", subsys->hw_desc_dir);
        goto end;
    }

    //读取端口
    rc = yaml_parse_ports(global_yaml_handle, subsys->name);

    if (0 != rc) {
        VLOG_ERR("yaml_parse_ports failed: %s", subsys->hw_desc_dir);
        goto end;
    }

    //为SFPP端口创建额外的a2设备
    // pm_create_a2_devices（）;

    //发送i2c初始化命令
    yaml_init_devices(global_yaml_handle, subsys->name);

end:
    //可能会尝试清理yaml句柄错误，但应用程序是
         //要中止，所以没有太多的意义。

    return rc;
}
