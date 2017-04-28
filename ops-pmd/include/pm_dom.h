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
 * Header file for pluggable module DOM information.
 ***************************************************************************/

#ifndef _DOM_H_
#define _DOM_H_


#define PASSWORD_LEN                4

#define SINGLE_PRECISION_FLOATING_POINT_DATA_LEN    4
#define FIXED_DECIMAL_DATA_LEN                      2


//
//
//      SFP Diagnostics Information
//
//

// SFP DOM status and control bits
typedef struct {
        // byte 110
        unsigned char data_ready_bar:1;
        unsigned char los:1;
        unsigned char tx_fault:1;
        unsigned char soft_rx_rate_select:1;
        unsigned char rx_rate_select_state:1;
        unsigned char reserved_110_5:1;
        unsigned char soft_tx_disable:1;
        unsigned char tx_disable_status:1;

        // byte 111
        unsigned char reserved_111;
} pm_sfp_status_control_bits_t;

// Alarm and warning flags
typedef struct {
        // byte 112
        unsigned char tx_pwr_low_alarm:1;
        unsigned char tx_pwr_high_alarm:1;
        unsigned char tx_bias_low_alarm:1;
        unsigned char tx_bias_high_alarm:1;
        unsigned char vcc_low_alarm:1;
        unsigned char vcc_high_alarm:1;
        unsigned char temp_low_alarm:1;
        unsigned char temp_high_alarm:1;

        unsigned char reserved_113_0:1;
        unsigned char reserved_113_1:1;
        unsigned char reserved_113_2:1;
        unsigned char reserved_113_3:1;
        unsigned char reserved_113_4:1;
        unsigned char reserved_113_5:1;
        unsigned char rx_pwr_low_alarm:1;
        unsigned char rx_pwr_high_alarm:1;

        unsigned char reserved_114;
        unsigned char reserved_115;

        unsigned char tx_pwr_low_warning:1;
        unsigned char tx_pwr_high_warning:1;
        unsigned char tx_bias_low_warning:1;
        unsigned char tx_bias_high_warning:1;
        unsigned char vcc_low_warning:1;
        unsigned char vcc_high_warning:1;
        unsigned char temp_low_warning:1;
        unsigned char temp_high_warning:1;

        unsigned char reserved_117_0:1;
        unsigned char reserved_117_1:1;
        unsigned char reserved_117_2:1;
        unsigned char reserved_117_3:1;
        unsigned char reserved_117_4:1;
        unsigned char reserved_117_5:1;
        unsigned char rx_pwr_low_warning:1;
        unsigned char rx_pwr_high_warning:1;

        unsigned char reserved_118;
        unsigned char reserved_119;
} pm_sfp_alarm_warning_bits_t;

typedef struct {
        unsigned char reserved_120;
        unsigned char reserved_121;
        unsigned char reserved_122;

        unsigned char password_byte_3;
        unsigned char password_byte_2;
        unsigned char password_byte_1;
        unsigned char password_byte_0;
} pm_sfp_password_t;

// SFP Diagnostics Information A2 page byte 0 to 127
typedef struct {
        char temp_high_alarm_msb;
        unsigned char temp_high_alarm_lsb;

        char temp_low_alarm_msb;
        unsigned char temp_low_alarm_lsb;

        char temp_high_warning_msb;
        unsigned char temp_high_warning_lsb;

        char temp_low_warning_msb;
        unsigned char temp_low_warning_lsb;

        unsigned char voltage_high_alarm_msb;
        unsigned char voltage_high_alarm_lsb;

        unsigned char voltage_low_alarm_msb;
        unsigned char voltage_low_alarm_lsb;

        unsigned char voltage_high_warning_msb;
        unsigned char voltage_high_warning_lsb;

        unsigned char voltage_low_warning_msb;
        unsigned char voltage_low_warning_lsb;

        unsigned char bias_high_alarm_msb;
        unsigned char bias_high_alarm_lsb;

        unsigned char bias_low_alarm_msb;
        unsigned char bias_low_alarm_lsb;

        unsigned char bias_high_warning_msb;
        unsigned char bias_high_warning_lsb;

        unsigned char bias_low_warning_msb;
        unsigned char bias_low_warning_lsb;

        unsigned char tx_power_high_alarm_msb;
        unsigned char tx_power_high_alarm_lsb;

        unsigned char tx_power_low_alarm_msb;
        unsigned char tx_power_low_alarm_lsb;

        unsigned char tx_power_high_warning_msb;
        unsigned char tx_power_high_warning_lsb;

        unsigned char tx_power_low_warning_msb;
        unsigned char tx_power_low_warning_lsb;

        unsigned char rx_power_high_alarm_msb;
        unsigned char rx_power_high_alarm_lsb;

        unsigned char rx_power_low_alarm_msb;
        unsigned char rx_power_low_alarm_lsb;

        unsigned char rx_power_high_warning_msb;
        unsigned char rx_power_high_warning_lsb;

        unsigned char rx_power_low_warning_msb;
        unsigned char rx_power_low_warning_lsb;

        unsigned char reserved_40;
        unsigned char reserved_41;
        unsigned char reserved_42;
        unsigned char reserved_43;
        unsigned char reserved_44;
        unsigned char reserved_45;
        unsigned char reserved_46;
        unsigned char reserved_47;
        unsigned char reserved_48;
        unsigned char reserved_49;
        unsigned char reserved_50;
        unsigned char reserved_51;
        unsigned char reserved_52;
        unsigned char reserved_53;
        unsigned char reserved_54;
        unsigned char reserved_55;

        unsigned char rx_pwr4[SINGLE_PRECISION_FLOATING_POINT_DATA_LEN];
        unsigned char rx_pwr3[SINGLE_PRECISION_FLOATING_POINT_DATA_LEN];
        unsigned char rx_pwr2[SINGLE_PRECISION_FLOATING_POINT_DATA_LEN];
        unsigned char rx_pwr1[SINGLE_PRECISION_FLOATING_POINT_DATA_LEN];
        unsigned char rx_pwr0[SINGLE_PRECISION_FLOATING_POINT_DATA_LEN];

        unsigned char current_tx_slope[FIXED_DECIMAL_DATA_LEN];
        unsigned char current_tx_offset[FIXED_DECIMAL_DATA_LEN];

        unsigned char power_tx_slope[FIXED_DECIMAL_DATA_LEN];
        unsigned char power_tx_offset[FIXED_DECIMAL_DATA_LEN];

        unsigned char temperature_slope[FIXED_DECIMAL_DATA_LEN];
        unsigned char temperature_offset[FIXED_DECIMAL_DATA_LEN];

        unsigned char voltage_slope[FIXED_DECIMAL_DATA_LEN];
        unsigned char voltage_offset[FIXED_DECIMAL_DATA_LEN];

        unsigned char reserved_92;
        unsigned char reserved_93;
        unsigned char reserved_94;

        unsigned char checksum;

        char temperature_msb;
        unsigned char temperature_lsb;

        unsigned char vcc_msb;
        unsigned char vcc_lsb;

        unsigned char tx_bias_msb;
        unsigned char tx_bias_lsb;

        unsigned char tx_power_msb;
        unsigned char tx_power_lsb;

        unsigned char rx_power_msb;
        unsigned char rx_power_lsb;

        unsigned char reserved_106;
        unsigned char reserved_107;
        unsigned char reserved_108;
        unsigned char reserved_109;

        pm_sfp_status_control_bits_t status_control_bits;
        pm_sfp_alarm_warning_bits_t alarm_warning_bits;
        pm_sfp_password_t password;

        unsigned char eeprom_select;
} pm_sfp_dom_t;


//
//
//      QSFP diagnostics information (DOM)
//
//

// QSFP Identifer Structure
typedef struct {
    // byte 1
    unsigned char reserved_1;

    // byte 2
    unsigned char data_not_ready:1;
    unsigned char intl:1;
    unsigned char reserved_2_2:1;
    unsigned char reserved_2_3:1;
    unsigned char reserved_2_4:1;
    unsigned char reserved_2_5:1;
    unsigned char reserved_2_6:1;
    unsigned char reserved_2_7:1;
} pm_qsfp_status_indicators_t;

// QSFP Interrupt Flags
typedef struct {
    // byte 3
    unsigned char latched_rx1_los_indicator:1;
    unsigned char latched_rx2_los_indicator:1;
    unsigned char latched_rx3_los_indicator:1;
    unsigned char latched_rx4_los_indicator:1;
    unsigned char latched_tx1_los_indicator:1;
    unsigned char latched_tx2_los_indicator:1;
    unsigned char latched_tx3_los_indicator:1;
    unsigned char latched_tx4_los_indicator:1;

    // byte 4
    unsigned char latched_tx1_fault_indicator:1;
    unsigned char latched_tx2_fault_indicator:1;
    unsigned char latched_tx3_fault_indicator:1;
    unsigned char latched_tx4_fault_indicator:1;
    unsigned char reserved_4_4:1;
    unsigned char reserved_4_5:1;
    unsigned char reserved_4_6:1;
    unsigned char reserved_4_7:1;

    // byte 5
    unsigned char reserved_5;

    // byte 6
    unsigned char reserved_6_0:1;
    unsigned char reserved_6_1:1;
    unsigned char reserved_6_2:1;
    unsigned char reserved_6_3:1;
    unsigned char latched_temp_low_warning:1;
    unsigned char latched_temp_high_warning:1;
    unsigned char latched_temp_low_alarm:1;
    unsigned char latched_temp_high_alarm:1;

    // byte 7
    unsigned char reserved_7_0:1;
    unsigned char reserved_7_1:1;
    unsigned char reserved_7_2:1;
    unsigned char reserved_7_3:1;
    unsigned char latched_vcc_low_warning:1;
    unsigned char latched_vcc_high_warning:1;
    unsigned char latched_vcc_low_alarm:1;
    unsigned char latched_vcc_high_alarm:1;

    // byte 8
    unsigned char reserved_8;

    // byte 9
    unsigned char latched_rx2_power_low_warning:1;
    unsigned char latched_rx2_power_high_warning:1;
    unsigned char latched_rx2_power_low_alarm:1;
    unsigned char latched_rx2_power_high_alarm:1;
    unsigned char latched_rx1_power_low_warning:1;
    unsigned char latched_rx1_power_high_warning:1;
    unsigned char latched_rx1_power_low_alarm:1;
    unsigned char latched_rx1_power_high_alarm:1;

    // byte 10
    unsigned char latched_rx4_power_low_warning:1;
    unsigned char latched_rx4_power_high_warning:1;
    unsigned char latched_rx4_power_low_alarm:1;
    unsigned char latched_rx4_power_high_alarm:1;
    unsigned char latched_rx3_power_low_warning:1;
    unsigned char latched_rx3_power_high_warning:1;
    unsigned char latched_rx3_power_low_alarm:1;
    unsigned char latched_rx3_power_high_alarm:1;

    // byte 11
    unsigned char latched_tx2_bias_low_warning:1;
    unsigned char latched_tx2_bias_high_warning:1;
    unsigned char latched_tx2_bias_low_alarm:1;
    unsigned char latched_tx2_bias_high_alarm:1;
    unsigned char latched_tx1_bias_low_warning:1;
    unsigned char latched_tx1_bias_high_warning:1;
    unsigned char latched_tx1_bias_low_alarm:1;
    unsigned char latched_tx1_bias_high_alarm:1;

    // byte 12
    unsigned char latched_tx4_bias_low_warning:1;
    unsigned char latched_tx4_bias_high_warning:1;
    unsigned char latched_tx4_bias_low_alarm:1;
    unsigned char latched_tx4_bias_high_alarm:1;
    unsigned char latched_tx3_bias_low_warning:1;
    unsigned char latched_tx3_bias_high_warning:1;
    unsigned char latched_tx3_bias_low_alarm:1;
    unsigned char latched_tx3_bias_high_alarm:1;

    // byte 13 to 21
    unsigned char reserved_13;
    unsigned char reserved_14;
    unsigned char reserved_15;
    unsigned char reserved_16;
    unsigned char reserved_17;
    unsigned char reserved_18;
    unsigned char reserved_19;
    unsigned char reserved_20;
    unsigned char reserved_21;
} pm_qsfp_interrupt_flags_t;

// QSFP Module Monitors
typedef struct {
    // Internally measured module temperature
    char temp_msb; // byte 22
    unsigned char temp_lsb; // byte 23

    unsigned char reserved_24;
    unsigned char reserved_25;

    // Internally measured module supply voltage
    // bytes 26 and 27
    unsigned char voltage_msb;
    unsigned char voltage_lsb;

    unsigned char reserved_28;
    unsigned char reserved_29;
    unsigned char reserved_30;
    unsigned char reserved_31;
    unsigned char reserved_32;
    unsigned char reserved_33;
} pm_qsfp_module_monitors_t;

// QSFP Channel Monitors
typedef struct {
    // bytes 34 to 49
    unsigned char rx1_power_msb;
    unsigned char rx1_power_lsb;

    unsigned char rx2_power_msb;
    unsigned char rx2_power_lsb;

    unsigned char rx3_power_msb;
    unsigned char rx3_power_lsb;

    unsigned char rx4_power_msb;
    unsigned char rx4_power_lsb;

    unsigned char tx1_bias_msb;
    unsigned char tx1_bias_lsb;

    unsigned char tx2_bias_msb;
    unsigned char tx2_bias_lsb;

    unsigned char tx3_bias_msb;
    unsigned char tx3_bias_lsb;

    unsigned char tx4_bias_msb;
    unsigned char tx4_bias_lsb;

    unsigned char reserved_50;
    unsigned char reserved_51;
    unsigned char reserved_52;
    unsigned char reserved_53;
    unsigned char reserved_54;
    unsigned char reserved_55;
    unsigned char reserved_56;
    unsigned char reserved_57;

    unsigned char reserved_58;
    unsigned char reserved_59;
    unsigned char reserved_60;
    unsigned char reserved_61;
    unsigned char reserved_62;
    unsigned char reserved_63;
    unsigned char reserved_64;
    unsigned char reserved_65;

    unsigned char reserved_66;
    unsigned char reserved_67;
    unsigned char reserved_68;
    unsigned char reserved_69;
    unsigned char reserved_70;
    unsigned char reserved_71;
    unsigned char reserved_72;
    unsigned char reserved_73;

    unsigned char reserved_74;
    unsigned char reserved_75;
    unsigned char reserved_76;
    unsigned char reserved_77;
    unsigned char reserved_78;
    unsigned char reserved_79;
    unsigned char reserved_80;
    unsigned char reserved_81;
} pm_qsfp_channel_monitors_t;

// QSFP Control Structure
typedef struct {
    // byte 86
    unsigned char tx1_disable:1;
    unsigned char tx2_disable:1;
    unsigned char tx3_disable:1;
    unsigned char tx4_disable:1;

    unsigned char reserved_86_4:1;
    unsigned char reserved_86_5:1;
    unsigned char reserved_86_6:1;
    unsigned char reserved_86_7:1;

    // byte 87
    unsigned char rx1_rate_select_lsb:1;
    unsigned char rx1_rate_select_msb:1;
    unsigned char rx2_rate_select_lsb:1;
    unsigned char rx2_rate_select_msb:1;
    unsigned char rx3_rate_select_lsb:1;
    unsigned char rx3_rate_select_msb:1;
    unsigned char rx4_rate_select_lsb:1;
    unsigned char rx4_rate_select_msb:1;

    // byte 88
    unsigned char tx1_rate_select_lsb:1;
    unsigned char tx1_rate_select_msb:1;
    unsigned char tx2_rate_select_lsb:1;
    unsigned char tx2_rate_select_msb:1;
    unsigned char tx3_rate_select_lsb:1;
    unsigned char tx3_rate_select_msb:1;
    unsigned char tx4_rate_select_lsb:1;
    unsigned char tx4_rate_select_msb:1;

    // bytes 89 to 92
    unsigned char rx4_application_select;
    unsigned char rx3_application_select;
    unsigned char rx2_application_select;
    unsigned char rx1_application_select;

    // byte 93
    unsigned char power_override:1;
    unsigned char power_set:1;
    unsigned char reserved_93_2:1;
    unsigned char reserved_93_3:1;
    unsigned char reserved_93_4:1;
    unsigned char reserved_93_5:1;
    unsigned char reserved_93_6:1;
    unsigned char reserved_93_7:1;

    // byte 94 to 97
    unsigned char tx4_application_select;
    unsigned char tx3_application_select;
    unsigned char tx2_application_select;
    unsigned char tx1_application_select;
} pm_qsfp_control_t;

// QSFP Module & Channel Masks
typedef struct {
    // byte 100
    unsigned char masking_bit_rx1_los_indicator:1;
    unsigned char masking_bit_rx2_los_indicator:1;
    unsigned char masking_bit_rx3_los_indicator:1;
    unsigned char masking_bit_rx4_los_indicator:1;

    unsigned char masking_bit_tx1_los_indicator:1;
    unsigned char masking_bit_tx2_los_indicator:1;
    unsigned char masking_bit_tx3_los_indicator:1;
    unsigned char masking_bit_tx4_los_indicator:1;

    // byte 101
    unsigned char masking_bit_tx1_fault_indicator:1;
    unsigned char masking_bit_tx2_fault_indicator:1;
    unsigned char masking_bit_tx3_fault_indicator:1;
    unsigned char masking_bit_tx4_fault_indicator:1;

    unsigned char reserved_101_4:1;
    unsigned char reserved_101_5:1;
    unsigned char reserved_101_6:1;
    unsigned char reserved_101_7:1;

    unsigned char reserved_102;

    // byte 103
    unsigned char reserved_103_0:1;
    unsigned char reserved_103_1:1;
    unsigned char reserved_103_2:1;
    unsigned char reserved_103_3:1;

    unsigned char masking_bit_low_temp_warning:1;
    unsigned char masking_bit_high_temp_warning:1;
    unsigned char masking_bit_low_temp_alarm:1;
    unsigned char masking_bit_high_temp_alarm:1;

    // byte 104
    unsigned char reserved_104_0:1;
    unsigned char reserved_104_1:1;
    unsigned char reserved_104_2:1;
    unsigned char reserved_104_3:1;

    unsigned char masking_bit_low_vcc_warning:1;
    unsigned char masking_bit_high_vcc_warning:1;
    unsigned char masking_bit_low_vcc_alarm:1;
    unsigned char masking_bit_high_vcc_alarm:1;

    unsigned char reserved_105;
    unsigned char reserved_106;
} pm_qsfp_module_and_channel_masks_t;

// QSFP Password Change Entry Area
typedef struct {
    // bytes 119 to 122
    unsigned char password_change_entry_area[PASSWORD_LEN];
} pm_qsfp_password_change_entry_area_t;

// QSFP Password Entry Area
typedef struct {
    // bytes 123 to 126
    unsigned char password_entry_area[PASSWORD_LEN];
} pm_qsfp_password_entry_area_t;

// Lower Memory Map (byte 0 to 127)
typedef struct {
        unsigned char indentifier; // Identifer byte - 0
        pm_qsfp_status_indicators_t status; // Status bytes 1-2
        pm_qsfp_interrupt_flags_t interrupt_flags; // Interrupt Flags bytes 3-21
        pm_qsfp_module_monitors_t module_monitors;
        pm_qsfp_channel_monitors_t channel_monitors;
        unsigned char reserved_82[4];
        pm_qsfp_control_t control;
        unsigned char reserved_98[2];
        pm_qsfp_module_and_channel_masks_t module_and_channel_masks;
        unsigned char reserved_107[12];
        pm_qsfp_password_change_entry_area_t password_change_entry_area;
        pm_qsfp_password_entry_area_t password_entry_area;
        unsigned char page_select_btye;
} pm_qsfp_dom_t;


struct ovs_module_dom_info {
    char *temperature;
    char *vcc;

    char *temperature_high_alarm;
    char *temperature_low_alarm;
    char *temperature_high_warning;
    char *temperature_low_warning;
    char *vcc_high_alarm;
    char *vcc_low_alarm;
    char *vcc_high_warning;
    char *vcc_low_warning;

    char *temperature_high_alarm_threshold;
    char *temperature_low_alarm_threshold;
    char *temperature_high_warning_threshold;
    char *temperature_low_warning_threshold;
    char *vcc_high_alarm_threshold;
    char *vcc_low_alarm_threshold;
    char *vcc_high_warning_threshold;
    char *vcc_low_warning_threshold;

    // SFP Specific Variables
    char *tx_bias;
    char *rx_power;
    char *tx_power;

    char *tx_bias_high_alarm;
    char *tx_bias_low_alarm;
    char *tx_bias_high_warning;
    char *tx_bias_low_warning;
    char *tx_power_high_alarm;
    char *tx_power_low_alarm;
    char *tx_power_high_warning;
    char *tx_power_low_warning;
    char *rx_power_high_alarm;
    char *rx_power_low_alarm;
    char *rx_power_high_warning;
    char *rx_power_low_warning;

    char *tx_bias_high_alarm_threshold;
    char *tx_bias_low_alarm_threshold;
    char *tx_bias_high_warning_threshold;
    char *tx_bias_low_warning_threshold;
    char *tx_power_high_alarm_threshold;
    char *tx_power_low_alarm_threshold;
    char *tx_power_high_warning_threshold;
    char *tx_power_low_warning_threshold;
    char *rx_power_high_alarm_threshold;
    char *rx_power_low_alarm_threshold;
    char *rx_power_high_warning_threshold;
    char *rx_power_low_warning_threshold;

    // QSFP Secific Variables
    char *tx1_bias;
    char *tx2_bias;
    char *tx3_bias;
    char *tx4_bias;

    char *rx1_power;
    char *rx2_power;
    char *rx3_power;
    char *rx4_power;

    char *tx1_bias_high_alarm;
    char *tx1_bias_low_alarm;
    char *tx1_bias_high_warning;
    char *tx1_bias_low_warning;
    char *tx2_bias_high_alarm;
    char *tx2_bias_low_alarm;
    char *tx2_bias_high_warning;
    char *tx2_bias_low_warning;
    char *tx3_bias_high_alarm;
    char *tx3_bias_low_alarm;
    char *tx3_bias_high_warning;
    char *tx3_bias_low_warning;
    char *tx4_bias_high_alarm;
    char *tx4_bias_low_alarm;
    char *tx4_bias_high_warning;
    char *tx4_bias_low_warning;

    char *rx1_power_high_alarm;
    char *rx1_power_low_alarm;
    char *rx1_power_high_warning;
    char *rx1_power_low_warning;
    char *rx2_power_high_alarm;
    char *rx2_power_low_alarm;
    char *rx2_power_high_warning;
    char *rx2_power_low_warning;
    char *rx3_power_high_alarm;
    char *rx3_power_low_alarm;
    char *rx3_power_high_warning;
    char *rx3_power_low_warning;
    char *rx4_power_high_alarm;
    char *rx4_power_low_alarm;
    char *rx4_power_high_warning;
    char *rx4_power_low_warning;

    char *tx1_bias_high_alarm_threshold;
    char *tx1_bias_low_alarm_threshold;
    char *tx1_bias_high_warning_threshold;
    char *tx1_bias_low_warning_threshold;
    char *tx2_bias_high_alarm_threshold;
    char *tx2_bias_low_alarm_threshold;
    char *tx2_bias_high_warning_threshold;
    char *tx2_bias_low_warning_threshold;
    char *tx3_bias_high_alarm_threshold;
    char *tx3_bias_low_alarm_threshold;
    char *tx3_bias_high_warning_threshold;
    char *tx3_bias_low_warning_threshold;
    char *tx4_bias_high_alarm_threshold;
    char *tx4_bias_low_alarm_threshold;
    char *tx4_bias_high_warning_threshold;
    char *tx4_bias_low_warning_threshold;

    char *rx1_power_high_alarm_threshold;
    char *rx1_power_low_alarm_threshold;
    char *rx1_power_high_warning_threshold;
    char *rx1_power_low_warning_threshold;
    char *rx2_power_high_alarm_threshold;
    char *rx2_power_low_alarm_threshold;
    char *rx2_power_high_warning_threshold;
    char *rx2_power_low_warning_threshold;
    char *rx3_power_high_alarm_threshold;
    char *rx3_power_low_alarm_threshold;
    char *rx3_power_high_warning_threshold;
    char *rx3_power_low_warning_threshold;
    char *rx4_power_high_alarm_threshold;
    char *rx4_power_low_alarm_threshold;
    char *rx4_power_high_warning_threshold;
    char *rx4_power_low_warning_threshold;
};

#endif
