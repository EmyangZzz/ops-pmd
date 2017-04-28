/*
 *  (c) Copyright 2016 Hewlett Packard Enterprise Development LP
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
 * Source file for pluggable module DOM information extraction.
 ***************************************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>

#include <vswitch-idl.h>
#include <openswitch-idl.h>

#include "pmd.h"
#include "plug.h"
#include "pm_dom.h"

VLOG_DEFINE_THIS_MODULE(dom);

/*
  * set_a2_read_request：如果DOM信息存在且集成，则设置a2_read_requested
  */
void
set_a2_read_request(pm_port_t *port, pm_sfp_serial_id_t *serial_datap)
{
    if (0 == strcmp(port->module_device->connector, CONNECTOR_SFP_PLUS)) {
        if (serial_datap->diag_monitor_type.implemented_digital &&
                serial_datap->diag_monitor_type.internally_calibrated &&
                serial_datap->diag_monitor_type.power_measurement_type &&
                !serial_datap->diag_monitor_type.addr_change_required) {
            port->a2_read_requested = true;
            VLOG_DBG("sfpp serial id data indicates that the DOM info is present");
        }
    } else if ((0 == strcmp(port->module_device->connector,
                            CONNECTOR_QSFP_PLUS)) ||
               (0 == strcmp(port->module_device->connector,
                            CONNECTOR_QSFP28))) {
        pm_qsfp_serial_id_t *qsfpp_serial_id;

        qsfpp_serial_id = (pm_qsfp_serial_id_t *)serial_datap;

        if (qsfpp_serial_id->diag_monitor_type.average_input_optical_power) {
            port->a2_read_requested = true;
            VLOG_DBG("qsfpp serial id data indicates that the DOM info is present");
        }
    }
}


/*
  * pm_set_a2：设置a2值（强制，因为它是按需）
  */
void
pm_set_a2(pm_port_t *port, pm_sfp_dom_t *a2_data)
{
    int type;
    float temperature, temp_high_alarm, temp_low_alarm,
                       temp_high_warning, temp_low_warning,
          vcc, voltage_high_alarm, voltage_low_alarm,
               voltage_high_warning, voltage_low_warning,
          tx_bias, bias_high_alarm, bias_low_alarm,
                   bias_high_warning, bias_low_warning,
          rx_power, rx_power_high_alarm, rx_power_low_alarm,
                    rx_power_high_warning, rx_power_low_warning,
          tx_power, tx_power_high_alarm, tx_power_low_alarm,
                    tx_power_high_warning, tx_power_low_warning,
          tx1_bias, tx2_bias, tx3_bias, tx4_bias,
          rx1_power, rx2_power, rx3_power, rx4_power;
    pm_qsfp_dom_t *qsfp_a2_data;

    //忽略不可插拔的模块
    if (false == port->module_device->pluggable) {
        VLOG_DBG("port is not pluggable: %s", port->instance);
        return;
    }

    //忽略没有连接器数据的模块
    if (NULL == port->module_device->connector) {
        VLOG_WARN("no connector info for port: %s", port->instance);
        return;
    }

    //准备处理SFP +和QSFP不同
    if (strcmp(port->module_device->connector, CONNECTOR_SFP_PLUS) == 0) {
        type = MODULE_TYPE_SFP_PLUS;
    } else if (strcmp(port->module_device->connector, CONNECTOR_QSFP_PLUS) == 0) {
        type = MODULE_TYPE_QSFP_PLUS;
    } else if (strcmp(port->module_device->connector, CONNECTOR_QSFP28) == 0) {
        type = MODULE_TYPE_QSFP28;
    } else {
        VLOG_WARN("unknown connector type for port: %s (%s)",
                  port->instance, port->module_device->connector);

        // to-do：当连接器类型未知时，删除dom信息
                 // pm_delete_all_dom_data（port）;
        SET_STATIC_STRING(port, connector, OVSREC_INTERFACE_PM_INFO_CONNECTOR_UNKNOWN);
        return;
    }

    switch (type) {
        case MODULE_TYPE_SFP_PLUS:
          //解析温度值
            temperature = (a2_data->temperature_msb +
                          (float)(a2_data->temperature_lsb/256));
            SET_FLOAT_STRING(port, temperature, temperature);

            SET_BOOL_STRING(port, temperature_high_alarm,
                            a2_data->alarm_warning_bits.temp_high_alarm);
            SET_BOOL_STRING(port, temperature_low_alarm,
                            a2_data->alarm_warning_bits.temp_low_alarm);
            SET_BOOL_STRING(port, temperature_high_warning,
                            a2_data->alarm_warning_bits.temp_high_warning);
            SET_BOOL_STRING(port, temperature_low_warning,
                            a2_data->alarm_warning_bits.temp_low_warning);

            temp_high_alarm = (a2_data->temp_high_alarm_msb +
                              (float)(a2_data->temp_high_alarm_lsb/256));
            SET_FLOAT_STRING(port, temperature_high_alarm_threshold,
                             temp_high_alarm);

            temp_low_alarm = (a2_data->temp_low_alarm_msb +
                             (float)(a2_data->temp_low_alarm_lsb/256));
            SET_FLOAT_STRING(port, temperature_low_alarm_threshold,
                             temp_low_alarm);

            temp_high_warning = (a2_data->temp_high_warning_msb +
                                (float)(a2_data->temp_high_warning_lsb/256));
            SET_FLOAT_STRING(port, temperature_high_warning_threshold,
                             temp_high_warning);

            temp_low_warning = (a2_data->temp_low_warning_msb +
                               (float)(a2_data->temp_low_warning_lsb/256));
            SET_FLOAT_STRING(port, temperature_low_warning_threshold,
                             temp_low_warning);


            //解析Vcc值
            vcc = (float) ((a2_data->vcc_msb<<8) |
                  (a2_data->vcc_lsb)) * 0.0001;
            SET_FLOAT_STRING(port, vcc, vcc);

            SET_BOOL_STRING(port, vcc_high_alarm,
                            a2_data->alarm_warning_bits.vcc_high_alarm);
            SET_BOOL_STRING(port, vcc_low_alarm,
                            a2_data->alarm_warning_bits.vcc_low_alarm);
            SET_BOOL_STRING(port, vcc_high_warning,
                            a2_data->alarm_warning_bits.vcc_high_warning);
            SET_BOOL_STRING(port, vcc_low_warning,
                            a2_data->alarm_warning_bits.vcc_low_warning);

            voltage_high_alarm = (float) ((a2_data->voltage_high_alarm_msb<<8) |
                                 (a2_data->voltage_high_alarm_lsb)) * 0.0001;
            SET_FLOAT_STRING(port, vcc_high_alarm_threshold,
                             voltage_high_alarm);

            voltage_low_alarm = (float) ((a2_data->voltage_low_alarm_msb<<8) |
                                (a2_data->voltage_low_alarm_lsb)) * 0.0001;
            SET_FLOAT_STRING(port, vcc_low_alarm_threshold, voltage_low_alarm);

            voltage_high_warning = (float) ((a2_data->voltage_high_warning_msb<<8) |
                                   (a2_data->voltage_high_warning_lsb)) * 0.0001;
            SET_FLOAT_STRING(port, vcc_high_warning_threshold, voltage_high_warning);

            voltage_low_warning = (float) ((a2_data->voltage_low_warning_msb<<8) |
                                  (a2_data->voltage_low_warning_lsb)) * 0.0001;
            SET_FLOAT_STRING(port, vcc_low_warning_threshold, voltage_low_warning);


            //解析tx_bias
            tx_bias = (float) (a2_data->tx_bias_msb<<8 | a2_data->tx_bias_lsb) * 0.002;
            SET_FLOAT_STRING(port, tx_bias, tx_bias);

            SET_BOOL_STRING(port, tx_bias_high_alarm,
                            a2_data->alarm_warning_bits.tx_bias_high_alarm);
            SET_BOOL_STRING(port, tx_bias_low_alarm,
                            a2_data->alarm_warning_bits.tx_bias_low_alarm);
            SET_BOOL_STRING(port, tx_bias_high_warning,
                            a2_data->alarm_warning_bits.tx_bias_high_warning);
            SET_BOOL_STRING(port, tx_bias_low_warning,
                            a2_data->alarm_warning_bits.tx_bias_low_warning);

            bias_high_alarm = (float) (a2_data->bias_high_alarm_msb<<8 |
                              a2_data->bias_high_alarm_lsb) * 0.002;
            SET_FLOAT_STRING(port, tx_bias_high_alarm_threshold, bias_high_alarm);

            bias_low_alarm = (float) (a2_data->bias_low_alarm_msb<<8 |
                             a2_data->bias_low_alarm_lsb) * 0.002;
            SET_FLOAT_STRING(port, tx_bias_low_alarm_threshold, bias_low_alarm);

            bias_high_warning = (float) (a2_data->bias_high_warning_msb<<8 |
                                a2_data->bias_high_warning_lsb) * 0.002;
            SET_FLOAT_STRING(port, tx_bias_high_warning_threshold, bias_high_warning);

            bias_low_warning = (float) (a2_data->bias_low_warning_msb<<8 |
                               a2_data->bias_low_warning_lsb) * 0.002;
            SET_FLOAT_STRING(port, tx_bias_low_warning_threshold, bias_low_warning);


            //解析rx_power
            rx_power = (float) (a2_data->rx_power_msb<<8 | a2_data->rx_power_lsb) * 0.0001;
            SET_FLOAT_STRING(port, rx_power, rx_power);

            SET_BOOL_STRING(port, rx_power_high_alarm,
                            a2_data->alarm_warning_bits.rx_pwr_high_alarm);
            SET_BOOL_STRING(port, rx_power_low_alarm,
                            a2_data->alarm_warning_bits.rx_pwr_low_alarm);
            SET_BOOL_STRING(port, rx_power_high_warning,
                            a2_data->alarm_warning_bits.rx_pwr_high_warning);
            SET_BOOL_STRING(port, rx_power_low_warning,
                            a2_data->alarm_warning_bits.rx_pwr_low_warning);

            rx_power_high_alarm = (float) (a2_data->rx_power_high_alarm_msb<<8 |
                                  a2_data->rx_power_high_alarm_lsb) * 0.0001;
            SET_FLOAT_STRING(port, rx_power_high_alarm_threshold, rx_power_high_alarm);

            rx_power_low_alarm = (float) (a2_data->rx_power_low_alarm_msb<<8 |
                                 a2_data->rx_power_low_alarm_lsb) * 0.0001;
            SET_FLOAT_STRING(port, rx_power_low_alarm_threshold, rx_power_low_alarm);

            rx_power_high_warning = (float) (a2_data->rx_power_high_warning_msb<<8 |
                                    a2_data->rx_power_high_warning_lsb) * 0.0001;
            SET_FLOAT_STRING(port, rx_power_high_warning_threshold, rx_power_high_warning);

            rx_power_low_warning = (float) (a2_data->rx_power_low_warning_msb<<8 |
                                   a2_data->rx_power_low_warning_lsb) * 0.0001;
            SET_FLOAT_STRING(port, rx_power_low_warning_threshold, rx_power_low_warning);


            //解析tx_power
            tx_power = (float) (a2_data->tx_power_msb<<8 | a2_data->tx_power_lsb) * 0.0001;
            SET_FLOAT_STRING(port, tx_power, tx_power);

            SET_BOOL_STRING(port, tx_power_high_alarm,
                            a2_data->alarm_warning_bits.tx_pwr_high_alarm);
            SET_BOOL_STRING(port, tx_power_low_alarm,
                            a2_data->alarm_warning_bits.tx_pwr_low_alarm);
            SET_BOOL_STRING(port, tx_power_high_warning,
                            a2_data->alarm_warning_bits.tx_pwr_high_warning);
            SET_BOOL_STRING(port, tx_power_low_warning,
                            a2_data->alarm_warning_bits.tx_pwr_low_warning);

            tx_power_high_alarm = (float) (a2_data->tx_power_high_alarm_msb<<8 |
                                   a2_data->tx_power_high_alarm_lsb) * 0.0001;
            SET_FLOAT_STRING(port, tx_power_high_alarm_threshold, tx_power_high_alarm);

            tx_power_low_alarm = (float) (a2_data->tx_power_low_alarm_msb<<8 |
                                 a2_data->tx_power_low_alarm_lsb) * 0.0001;
            SET_FLOAT_STRING(port, tx_power_low_alarm_threshold, tx_power_low_alarm);

            tx_power_high_warning = (float) (a2_data->tx_power_high_warning_msb<<8 |
                                    a2_data->tx_power_high_warning_lsb) * 0.0001;
            SET_FLOAT_STRING(port, tx_power_high_warning_threshold, tx_power_high_warning);

            tx_power_low_warning = (float) (a2_data->tx_power_low_warning_msb<<8 |
                                   a2_data->tx_power_low_warning_lsb) * 0.0001;
            SET_FLOAT_STRING(port, tx_power_low_warning_threshold, tx_power_low_warning);


            SET_BINARY(port, a2, (char *)a2_data, sizeof(pm_sfp_dom_t));
            break;
        case MODULE_TYPE_QSFP_PLUS:
        case MODULE_TYPE_QSFP28:
            qsfp_a2_data = (pm_qsfp_dom_t *) a2_data;

            //解析温度值
            temperature = (qsfp_a2_data->module_monitors.temp_msb +
                          (float)(qsfp_a2_data->module_monitors.temp_lsb/256));
            SET_FLOAT_STRING(port, temperature, temperature);

            //解析Vcc值
            vcc = (float) ((qsfp_a2_data->module_monitors.voltage_msb<<8) |
                           (qsfp_a2_data->module_monitors.voltage_lsb)) * 0.0001;
            SET_FLOAT_STRING(port, vcc, vcc);

            //每个车道分段偏置电流和接收功率
                         //
                           // Lane 1
                           //解析tx_bias
            tx1_bias = (float) (qsfp_a2_data->channel_monitors.tx1_bias_msb<<8 |
                                qsfp_a2_data->channel_monitors.tx1_bias_lsb) * 0.002;
            SET_FLOAT_STRING(port, tx1_bias, tx1_bias);

            SET_BOOL_STRING(port, tx1_bias_high_alarm,
                            qsfp_a2_data->interrupt_flags.latched_tx1_bias_high_alarm);
            SET_BOOL_STRING(port, tx1_bias_low_alarm,
                            qsfp_a2_data->interrupt_flags.latched_tx1_bias_low_alarm);
            SET_BOOL_STRING(port, tx1_bias_high_warning,
                            qsfp_a2_data->interrupt_flags.latched_tx1_bias_high_warning);
            SET_BOOL_STRING(port, tx1_bias_low_warning,
                            qsfp_a2_data->interrupt_flags.latched_tx1_bias_low_warning);

            //解析rx_power
            rx1_power = (float) (qsfp_a2_data->channel_monitors.rx1_power_msb<<8 |
                                 qsfp_a2_data->channel_monitors.rx1_power_lsb) * 0.0001;
            SET_FLOAT_STRING(port, rx1_power, rx1_power);

            SET_BOOL_STRING(port, rx1_power_high_alarm,
                            qsfp_a2_data->interrupt_flags.latched_rx1_power_high_alarm);
            SET_BOOL_STRING(port, rx1_power_low_alarm,
                            qsfp_a2_data->interrupt_flags.latched_rx1_power_low_alarm);
            SET_BOOL_STRING(port, rx1_power_high_warning,
                            qsfp_a2_data->interrupt_flags.latched_rx1_power_high_warning);
            SET_BOOL_STRING(port, rx1_power_low_warning,
                            qsfp_a2_data->interrupt_flags.latched_rx1_power_low_warning);

            // Lane 2
            //
            //解析tx_bias
            tx2_bias = (float) (qsfp_a2_data->channel_monitors.tx2_bias_msb<<8 |
                                qsfp_a2_data->channel_monitors.tx2_bias_lsb) * 0.002;
            SET_FLOAT_STRING(port, tx2_bias, tx2_bias);

            SET_BOOL_STRING(port, tx2_bias_high_alarm,
                            qsfp_a2_data->interrupt_flags.latched_tx2_bias_high_alarm);
            SET_BOOL_STRING(port, tx2_bias_low_alarm,
                            qsfp_a2_data->interrupt_flags.latched_tx2_bias_low_alarm);
            SET_BOOL_STRING(port, tx2_bias_high_warning,
                            qsfp_a2_data->interrupt_flags.latched_tx2_bias_high_warning);
            SET_BOOL_STRING(port, tx2_bias_low_warning,
                            qsfp_a2_data->interrupt_flags.latched_tx2_bias_low_warning);


            //解析rx_power
            rx2_power = (float) (qsfp_a2_data->channel_monitors.rx2_power_msb<<8 |
                                 qsfp_a2_data->channel_monitors.rx2_power_lsb) * 0.0001;
            SET_FLOAT_STRING(port, rx2_power, rx2_power);

            SET_BOOL_STRING(port, rx2_power_high_alarm,
                            qsfp_a2_data->interrupt_flags.latched_rx2_power_high_alarm);
            SET_BOOL_STRING(port, rx2_power_low_alarm,
                            qsfp_a2_data->interrupt_flags.latched_rx2_power_low_alarm);
            SET_BOOL_STRING(port, rx2_power_high_warning,
                            qsfp_a2_data->interrupt_flags.latched_rx2_power_high_warning);
            SET_BOOL_STRING(port, rx2_power_low_warning,
                            qsfp_a2_data->interrupt_flags.latched_rx2_power_low_warning);

            // Lane 3
            //
                         //解析tx_bias
            tx3_bias = (float) (qsfp_a2_data->channel_monitors.tx3_bias_msb<<8 |
                                qsfp_a2_data->channel_monitors.tx3_bias_lsb) * 0.002;
            SET_FLOAT_STRING(port, tx3_bias, tx3_bias);

            SET_BOOL_STRING(port, tx3_bias_high_alarm,
                            qsfp_a2_data->interrupt_flags.latched_tx3_bias_high_alarm);
            SET_BOOL_STRING(port, tx3_bias_low_alarm,
                            qsfp_a2_data->interrupt_flags.latched_tx3_bias_low_alarm);
            SET_BOOL_STRING(port, tx3_bias_high_warning,
                            qsfp_a2_data->interrupt_flags.latched_tx3_bias_high_warning);
            SET_BOOL_STRING(port, tx3_bias_low_warning,
                            qsfp_a2_data->interrupt_flags.latched_tx3_bias_low_warning);


            // Parsing rx_power
            rx3_power = (float) (qsfp_a2_data->channel_monitors.rx3_power_msb<<8 |
                                 qsfp_a2_data->channel_monitors.rx3_power_lsb) * 0.0001;
            SET_FLOAT_STRING(port, rx3_power, rx3_power);

            SET_BOOL_STRING(port, rx3_power_high_alarm,
                            qsfp_a2_data->interrupt_flags.latched_rx3_power_high_alarm);
            SET_BOOL_STRING(port, rx3_power_low_alarm,
                            qsfp_a2_data->interrupt_flags.latched_rx3_power_low_alarm);
            SET_BOOL_STRING(port, rx3_power_high_warning,
                            qsfp_a2_data->interrupt_flags.latched_rx3_power_high_warning);
            SET_BOOL_STRING(port, rx3_power_low_warning,
                            qsfp_a2_data->interrupt_flags.latched_rx3_power_low_warning);


            // Lane 4
            //
            // Parsing tx_bias
            tx4_bias = (float) (qsfp_a2_data->channel_monitors.tx4_bias_msb<<8 |
                                qsfp_a2_data->channel_monitors.tx4_bias_lsb) * 0.002;
            SET_FLOAT_STRING(port, tx4_bias, tx4_bias);

            SET_BOOL_STRING(port, tx4_bias_high_alarm,
                            qsfp_a2_data->interrupt_flags.latched_tx4_bias_high_alarm);
            SET_BOOL_STRING(port, tx4_bias_low_alarm,
                            qsfp_a2_data->interrupt_flags.latched_tx4_bias_low_alarm);
            SET_BOOL_STRING(port, tx4_bias_high_warning,
                            qsfp_a2_data->interrupt_flags.latched_tx4_bias_high_warning);
            SET_BOOL_STRING(port, tx4_bias_low_warning,
                            qsfp_a2_data->interrupt_flags.latched_tx4_bias_low_warning);


            // Parsing rx_power
            rx4_power = (float) (qsfp_a2_data->channel_monitors.rx4_power_msb<<8 |
                                 qsfp_a2_data->channel_monitors.rx4_power_lsb) * 0.0001;
            SET_FLOAT_STRING(port, rx4_power, rx4_power);

            SET_BOOL_STRING(port, rx4_power_high_alarm,
                            qsfp_a2_data->interrupt_flags.latched_rx4_power_high_alarm);
            SET_BOOL_STRING(port, rx4_power_low_alarm,
                            qsfp_a2_data->interrupt_flags.latched_rx4_power_low_alarm);
            SET_BOOL_STRING(port, rx4_power_high_warning,
                            qsfp_a2_data->interrupt_flags.latched_rx4_power_high_warning);
            SET_BOOL_STRING(port, rx4_power_low_warning,
                            qsfp_a2_data->interrupt_flags.latched_rx4_power_low_warning);


            SET_BINARY(port, a2, (char *)qsfp_a2_data, sizeof(pm_qsfp_dom_t));
            break;
    }
}
