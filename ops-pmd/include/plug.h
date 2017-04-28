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
 * Header file for the Pluggable Module Daemon
 ***************************************************************************/

#ifndef _PLUG_H_
#define _PLUG_H_

// field size defnitions
#define PM_VENDOR_NAME_LEN 16
#define PM_VENDOR_OUI_LEN 3
#define PM_VENDOR_PN_LEN 16
#define PM_VENDOR_SN_LEN 16

// SFP+ and QSFP definitions for VENDOR_REV_LEN are different
#define PM_SFP_VENDOR_REV_LEN   4
#define PM_QSFP_VENDOR_REV_LEN  2

#define PM_SERIAL_ID_VENDOR_AREA_LEN 32

#define PM_CONNECTOR_COPPER_PIGTAIL 0x21

#define SFP_SERIAL_ID_OFFSET        0
#define QSFP_SERIAL_ID_OFFSET       128

#define QSFP_DISABLE_OFFSET         86

typedef struct pm_date_code {
        unsigned char   year[2];        // Year since 2000, 00-99 (in ASCII)
        unsigned char   month[2];       // Month in digits, 01-12 (in ASCII)
        unsigned char   day[2];         // Day of Month, 01-31 (in ASCII)
        unsigned char   lot_code[2];    // Vendor Specific Lot Code (in ASCII)
} pm_date_code_t;

// OPS_TODO: USING BIT MAP DATA STRUCTURES IS NOT PORTABLE
// OPS_TODO: CHANGE THIS IMPLEMENTATION TO USE #define VALUES

//
//
//
//          SFP+ DEFINITIONS
//
//
//


//      SFP Transceiver Code Structure
typedef struct {
        /* byte 3 */
                        // Infiniband Compliance Codes
        unsigned char   ib_1x_copper_passive:1;                 // SFP+ Only
        unsigned char   ib_1x_copper_active:1;                  // SFP+ Only
        unsigned char   ib_1x_lx:1;                             // SFP+ Only
        unsigned char   ib_1x_sx:1;                             // SFP+ Only
                        // 10G Ethernet Compliance Codes
        unsigned char   enet_10gbase_sr:1;                      // SFP+ Only
        unsigned char   enet_10gbase_lr:1;                      // SFP+ Only
        unsigned char   enet_10gbase_lrm:1;                     // SFP+ Only
        unsigned char   enet_10gbase_er:1;                      // SFP+ Only

        /* byte 4 */
                        // SONET Compliance Codes
        unsigned char   oc_48_short:1;
        unsigned char   oc_48_intermediate:1;
        unsigned char   oc_48_long:1;
        unsigned char   sonet_reach_specifier:2;                // SFP+ Only
        unsigned char   oc_192:1;                               // SFP+ Only
                        // ESCON Compliance Codes
        unsigned char   escon_smf_1310nm_laser:1;               // SFP+ Only
        unsigned char   escon_mmf_1310nm_led:1;                 // SFP+ Only

        /* byte 5 */
        unsigned char   oc_3_multi_short:1;
        unsigned char   oc_3_single_intermdiate:1;
        unsigned char   oc_3_single_long:1;
        unsigned char   reserved_4:1;
        unsigned char   oc_12_multi_short:1;
        unsigned char   oc_12_single_intermdiate:1;
        unsigned char   oc_12_single_long:1;
        unsigned char   reserved_3:1;

        /* byte 6 */
                        // Ethernet Compliance Codes
        unsigned char   enet_1000base_sx:1;
        unsigned char   enet_1000base_lx:1;
        unsigned char   enet_1000base_cx:1;
        unsigned char   enet_1000base_t:1;
        unsigned char   enet_100base_lx_lx10:1;                 // SFP+ Only
        unsigned char   enet_100base_fx:1;                      // SFP+ Only
        unsigned char   enet_base_bx10:1;                       // SFP+ Only
        unsigned char   enet_base_px:1;                         // SFP+ Only

        /* byte 7 */
                        // Fiber Channel Transmitter Technology
        unsigned char   fc_electrical_inter_enclosure:1;
        unsigned char   fc_longwave_laser_lc:1;
        unsigned char   fc_shortwave_laser_linear_rx:1;         // SFP+ Only
                        // Fiber Channel Link Length
        unsigned char   fc_length_medium:1;                     // SFP+ Only
        unsigned char   fc_length_long:1;
        unsigned char   fc_length_intermediate:1;
        unsigned char   fc_length_short:1;
        unsigned char   fc_length_very_long:1;

        /* byte 8 */
                        // Unallocated
        unsigned char   reserved_7:1;
        unsigned char   reserved_7_1:1;
                        // SFP+ Cable Technology
        unsigned char   cable_technology_passive:1;             // SFP+ Only
        unsigned char   cable_technology_active:1;              // SFP+ Only
        unsigned char   fc_longwave_laser_ll:1;
        unsigned char   fc_shortwave_laser_ofc:1;
        unsigned char   fc_shortwave_laser:1;
        unsigned char   fc_electrical_intra_enclosure:1;

        /* byte 9 */
                        // Fiber Channel Transmission Media
        unsigned char   fc_media_single_mode:1;
        unsigned char   reserved_8:1;
        unsigned char   fc_media_multi_mode_5000cm:1;
        unsigned char   fc_media_multi_mode_6250cm:1;
        unsigned char   fc_media_video_coax:1;
        unsigned char   fc_media_miniature_coax:1;
        unsigned char   fc_media_shielded_twisted_pair:1;
        unsigned char   fc_media_twin_axial_pair:1;

        /* byte 10 */
                        // Fiber Channel Speed
        unsigned char   fc_speed_100:1;
        unsigned char   reserved_11:1;
        unsigned char   fc_speed_200:1;
        unsigned char   fc_speed_3200:1;
        unsigned char   fc_speed_400:1;
        unsigned char   fc_speed_1600:1;                        // SFP+ Only
        unsigned char   fc_speed_800:1;                         // SFP+ Only
        unsigned char   fc_speed_1200:1;                        // SFP+ Only
} pm_sfp_serial_id_transceiver_t;

//      SFP Extended Options Structure
typedef struct {
        /* byte 0 */
        unsigned char   linear_receiver_output:1; // SFP+, SFF-8431, Linear Out
        unsigned char   power_class:1;          // SFP+, SFF-8431, Power class
        unsigned char   cooled_transceiver:1;   // SFP+, SFF-8431, Cooled
        unsigned char   retimer_or_cdr:1;
        unsigned char   paging_implementation:1;    // A2/127d is page sellect
        unsigned char   high_power_level_declaration:1; // SFF-8431 Addendum
        unsigned char   reserved_0:2;
        /* byte 1 */
        unsigned char   reserved_2:1;
        unsigned char   loss_of_signal:1;       // LOS is implemented
        unsigned char   loss_of_signal_inverted:1; // LOS is implemented
        unsigned char   tx_fault:1;             // TX_FAULT is implemented
        unsigned char   tx_disable:1;           // TX_DISABLE is implemented
        unsigned char   rate_select:1;          // RATE_SELECT is implemented
        unsigned char   tunable_transmitter:1;  // SFF-8690
        unsigned char   receiver_decision_threshold:1;
} pm_sfp_options_t;

//      SFP+ Diagnostic Monitoring Type Structure
typedef struct {
        unsigned char   reserved:2;
        unsigned char   addr_change_required:1; // Diag Page not at 0xA2
        unsigned char   power_measurement_type:1; // 0 = OMA; 1 = Avg power
        unsigned char   externally_calibrated:1; // Externally calibrated
        unsigned char   internally_calibrated:1; // Internally calibrated
        unsigned char   implemented_digital:1;  // Digital Diag Monitoring
        unsigned char   legacy:1;               // Legacy Diags (not SFP+)
} pm_sfp_diag_monitoring_type_t;

//      SFP+ Enhanced Options Structure
typedef struct {
        unsigned char   reserved:1;
        unsigned char   sff8431_rate_select:1;  // SFF-8431 Rate Select control
        unsigned char   sff8079_appl_select:1;  // SFF-8079 Appl Select control
        unsigned char   soft_rate_select:1;     // Soft RATE_SELECT monitoring
        unsigned char   soft_rx_los:1;          // Soft RX_LOS monitoring
        unsigned char   soft_tx_fault:1;        // Soft TX_FAULT monitoring
        unsigned char   soft_tx_disable:1;      // Soft TX_DISABLE ctrl/monitor
        unsigned char   alarm_warning_flags:1;  // Alarm/warning flag impl.
} pm_sfp_enhanced_options_t;


typedef struct {
        // Base ID fields (64 bytes, 0 to 63)
        unsigned char   identifier;             // Type of serial transceiver
        unsigned char   ext_identifier;         // Extended ID of serial trans.
        unsigned char   connector;              // Code for connector type
        pm_sfp_serial_id_transceiver_t transceiver; // Transceiver Coding
        unsigned char   encoding;               // Serial Encoding Algorithm
        unsigned char   bit_rate_nominal;       // in units of 100Mb/s
        unsigned char   rate_identifier;        // SFP+, Rate Select Func.
        unsigned char   length_smf_9um_in_km;   // 9um SM fiber, units of km
        unsigned char   length_smf_9um_in_100m; // 9um SM fiber, units of 100m
        unsigned char   length_mmf_50um_om2;    // 50um OM2 MM fiber, units 10m
        unsigned char   length_mmf_62_5um_om1;  // 62.5um OM1 MM fib, units 10m
        unsigned char   length_copper;          // copper, units of m
        unsigned char   length_mmf_50um_om3;    // SFP+, 50um OM3 MM, units 10m

        unsigned char   vendor_name[PM_VENDOR_NAME_LEN];
                                                // Vendor (in ASCII)

        unsigned char   transceiver_2;          // Transceiver Coding area 2
        unsigned char   vendor_oui[PM_VENDOR_OUI_LEN];
                                                // Vendor IEEE company ID

        unsigned char   vendor_part_number[PM_VENDOR_PN_LEN];
                                                // Part number (in ASCII)

        unsigned char   vendor_revision[PM_SFP_VENDOR_REV_LEN];
                                                // Part revision no. (in ASCII)

        unsigned short  laser_wavelength;       // SFP+, laser wavelength, in nm
        unsigned char   reserved_3;
        unsigned char   check_code_for_base;    // Sum of bytes 0-62

        // Extended ID fields (32 bytes, 64 to 95)
        pm_sfp_options_t    options;        // Optional Signals Implemented
        unsigned char   bit_rate_max_margin;    // BR margin, in % above nominal
        unsigned char   bit_rate_min_margin;    // BR margin, in % below nominal

        unsigned char   vendor_serial_number[PM_VENDOR_SN_LEN];
                                                // Serial # (in ASCII)

        pm_date_code_t      vendor_date_code; // Manufacturing Date Code
        pm_sfp_diag_monitoring_type_t diag_monitor_type;    // SFP+ only
        pm_sfp_enhanced_options_t enhanced_options;         // SFP+ only
        unsigned char   sff8472_compliance;     // SFP+ only
        unsigned char   check_code_for_extended; // Sum of bytes 64-94

        unsigned char vendor_area[PM_SERIAL_ID_VENDOR_AREA_LEN];
} pm_sfp_serial_id_t;

//
//
//
//          QSFP DEFINITIONS
//
//
//


//      QSFP Extended Indentifier Bytes
typedef struct {
        /* byte 129 */
        unsigned char   reserved_129_1:2;
        unsigned char   cdr_present_in_rx:1;
        unsigned char   cdr_present_in_tx:1;
        unsigned char   clei_code_present:1;
        unsigned char   reserved_129_5:1;
        unsigned char   power_class:2;  // PM_QSFP_EXT_IDENTIFIER_POWE*
} pm_qsfp_ext_indentifier_t;

//      QSFP Specification Compliance Code Structure
typedef struct {
        /* byte 131 */
                // 10/40G/100G Ethernet Compliance Codes
        unsigned char   enet_40g_xlppi:1;
        unsigned char   enet_40gbase_lr4:1;
        unsigned char   enet_40gbase_sr4:1;
        unsigned char   enet_40gbase_cr4:1;
        unsigned char   enet_10gbase_sr:1;
        unsigned char   enet_10gbase_lr:1;
        unsigned char   enet_10gbase_lrm:1;
        unsigned char   enet_extended:1;
        /* byte 132 */
                // SONET Compliance Codes
        unsigned char   oc_48_short:1;
        unsigned char   oc_48_intermediate:1;
        unsigned char   oc_48_long:1;
        unsigned char   sonet_40g_otn:1;
        unsigned char   reserved_132_4:1;
        unsigned char   reserved_132_5:1;
        unsigned char   reserved_132_6:1;
        unsigned char   reserved_132_7:1;
        /* byte 133 */
                // SAS/SATA Compliance Codes
        unsigned char   reserved_133_0:1;
        unsigned char   reserved_133_1:1;
        unsigned char   reserved_133_2:1;
        unsigned char   reserved_133_3:1;
        unsigned char   sas_3_0g:1;
        unsigned char   sas_6_0g:1;
        unsigned char   reserved_133_6:1;
        unsigned char   reserved_133_7:1;
        /* byte 134 */
                // GB Ethernet Compliance Codes
        unsigned char   enet_1000base_sx:1;
        unsigned char   enet_1000base_lx:1;
        unsigned char   enet_1000base_cx:1;
        unsigned char   enet_1000base_t:1;
        unsigned char   reserved_134_4:1;
        unsigned char   reserved_134_5:1;
        unsigned char   reserved_134_6:1;
        unsigned char   reserved_134_7:1;
        /* byte 135 */
                // Fiber Channel Transmitter Technology
        unsigned char   fc_electrical_inter_enclosure_el:1;
        unsigned char   fc_longwave_laser_lc:1;
        unsigned char   reserved_135_2:1;
                // Fiber Channel Link Length
        unsigned char   fc_length_medium:1;
        unsigned char   fc_length_long:1;
        unsigned char   fc_length_intermediate:1;
        unsigned char   fc_length_short:1;
        unsigned char   fc_length_very_long:1;
        /* byte 136 */
        unsigned char   reserved_136_0:1;
        unsigned char   reserved_136_1:1;
        unsigned char   reserved_136_2:1;
        unsigned char   reserved_136_3:1;
        unsigned char   fc_longwave_laser_ll:1;
        unsigned char   fc_shortwave_laser_ofc_sl:1;
        unsigned char   fc_shortwave_laser_sn:1;
        unsigned char   fc_electrical_intra_enclosure:1;
        /* byte 137 */
                // Fiber Channel Transmission Media
        unsigned char   fc_media_single_mode_sm:1;
        unsigned char   fc_media_multi_mode_50um_om3:1;
        unsigned char   fc_media_multi_mode_50m_m5:1;
        unsigned char   fc_media_multi_mode_6250cm_m6:1;
        unsigned char   fc_media_video_coax_tv:1;
        unsigned char   fc_media_miniature_coax_mi:1;
        unsigned char   fc_media_shielded_twisted_pair_tp:1;
        unsigned char   fc_media_twin_axial_pair_tw:1;
        /* byte 138 */
                // Fiber Channel Speed
        unsigned char   fc_speed_100:1;
        unsigned char   reserved_138_1:1;
        unsigned char   fc_speed_200:1;
        unsigned char   reserved_138_3:1;
        unsigned char   fc_speed_400:1;
        unsigned char   fc_speed_1600:1;
        unsigned char   fc_speed_800:1;
        unsigned char   fc_speed_1200:1;
} pm_qsfp_spec_compliance_t;

//      QSFP Extended Rate Select
typedef struct {
        /* byte 141 */
        unsigned char   version_1:1;
        unsigned char   reserved_141_1:1;
        unsigned char   reserved_141_2:1;
        unsigned char   reserved_141_3:1;
        unsigned char   reserved_141_4:1;
        unsigned char   reserved_141_5:1;
        unsigned char   reserved_141_6:1;
        unsigned char   reserved_141_7:1;
} pm_qsfp_rate_select_t;

//      QSFP Status Bytes
typedef struct {
        /* byte 147 */
        unsigned char   transmitter_tunable:1;
        unsigned char   apd_detector:1;
        unsigned char   cooled_transmitter:1;
        unsigned char   active_wavelength_control:1;
        unsigned char   transmitter_tech:4; // PM_QSFP_TRANSMITTER_TECH_*
} pm_qsfp_device_tech_t;

// Values for pm_qsfp_device_tech.transmitter_tech
#define PM_QSFP_TRANSMITTER_TECH_850_nm_VCLEL               0x0
#define PM_QSFP_TRANSMITTER_TECH_1310_nm_VCLEL              0x1
#define PM_QSFP_TRANSMITTER_TECH_1550_nm_VCLEL              0x2
#define PM_QSFP_TRANSMITTER_TECH_1310_nm_FP                 0x3
#define PM_QSFP_TRANSMITTER_TECH_1310_nm_DFB                0x4
#define PM_QSFP_TRANSMITTER_TECH_1550_nm_DFB                0x5
#define PM_QSFP_TRANSMITTER_TECH_1310_nm_EML                0x6
#define PM_QSFP_TRANSMITTER_TECH_1550_nm_EML                0x7
#define PM_QSFP_TRANSMITTER_TECH_OTHERS                     0x8
#define PM_QSFP_TRANSMITTER_TECH_1490_nm_DFB                0x9
#define PM_QSFP_TRANSMITTER_TECH_COPPER_UNEQUALIZED         0xA
#define PM_QSFP_TRANSMITTER_TECH_COPPER_PASSIVE_EQUALIZED   0xB
#define PM_QSFP_TRANSMITTER_TECH_COPPER_BOTH_END_LIMITING   0xC
#define PM_QSFP_TRANSMITTER_TECH_COPPER_FAR_END_LIMITING    0xD
#define PM_QSFP_TRANSMITTER_TECH_COPPER_NEAR_END_LIMITING   0xE
#define PM_QSFP_TRANSMITTER_TECH_COPPER_ACTIVE_EQUALIZED    0xF

//      QSFP Specification Compliance Code Structure
typedef struct {
        /* byte 164 */
                // Infiniband Compliance Codes
        unsigned char   sdr:1;
        unsigned char   ddr:1;
        unsigned char   qdr:1;
        unsigned char   fdr:1;
        unsigned char   edr:1;
        unsigned char   reserved_164_5:1;
        unsigned char   reserved_164_6:1;
        unsigned char   reserved_164_7:1;
} pm_qsfp_ib_compliance_t;

typedef union {
        unsigned short  laser_wavelength;       // Laser wavelength, in 0.05nm
        struct {
                unsigned char   at_2_5_ghz;     // At 2.5GHz, units of 1dB
                unsigned char   at_5_0_ghz;     // At 5.0GHz, units of 1dB
        } copper_attenuation;
} pm_qsfp_wavelength_t;

//      QSFP Extended Options Structure
typedef struct {
        /* byte 192 */
                // Extended Ethernet Compliance Codes
        unsigned char   ext_compliance_code;    // PM_QSFP_EXT_COMPLIANCE_CODE_*
        /* byte 193 */
        unsigned char   rx_output_amplitude:1;  // RX output applitude implement
        unsigned char   reserved_193_1:1;
        unsigned char   reserved_193_2:1;
        unsigned char   reserved_193_3:1;
        unsigned char   reserved_193_4:1;
        unsigned char   reserved_193_5:1;
        unsigned char   reserved_193_6:1;
        unsigned char   reserved_193_7:1;
        /* byte 194 */
        unsigned char   tx_squelch:1;           // TX Squelch implemented
        unsigned char   tx_squelch_disable:1;   // TX Squelch Disable implement
        unsigned char   rx_output_disable:1;    // RX Output Disable capable
        unsigned char   rx_squelch_disable:1;   // RX Squelch Disable implement
        unsigned char   reserved_194_4:1;
        unsigned char   reserved_194_5:1;
        unsigned char   reserved_194_6:1;
        unsigned char   reserved_194_7:1;
        /* byte 195 */
        unsigned char   reserved_195_0:1;
        unsigned char   loss_of_signal:1;       // LOS implemented
        unsigned char   tx_squelch_pave:1;      // TX Squelch reduces PAVE
        unsigned char   tx_fault:1;             // TX_FAULT implemented
        unsigned char   tx_disable:1;           // TX_DISABLE implemented
        unsigned char   rate_select:1;          // RATE_SELECT implemented
        unsigned char   memory_page_1:1;        // Memory Page 1 present
        unsigned char   memory_page_2:1;        // Memory Page 2 present
} pm_qsfp_options_t;

// Values for pm_qsfp_options_t.extended_compliance_code
#define PM_QSFP_EXT_COMPLIANCE_CODE_UNSPECIFIED     0x00
#define PM_QSFP_EXT_COMPLIANCE_CODE_100GBASE_AOC    0x01
#define PM_QSFP_EXT_COMPLIANCE_CODE_100GBASE_SR4    0x02
#define PM_QSFP_EXT_COMPLIANCE_CODE_100GBASE_LR4    0x03
#define PM_QSFP_EXT_COMPLIANCE_CODE_100GBASE_ER4    0x04
#define PM_QSFP_EXT_COMPLIANCE_CODE_100GBASE_SR10   0x05
#define PM_QSFP_EXT_COMPLIANCE_CODE_100GBASE_CWDM4  0x06
#define PM_QSFP_EXT_COMPLIANCE_CODE_100GBASE_PSM4   0x07
#define PM_QSFP_EXT_COMPLIANCE_CODE_100GBASE_ACC    0x08
#define PM_QSFP_EXT_COMPLIANCE_CODE_100GBASE_CR4    0x0B
#define PM_QSFP_EXT_COMPLIANCE_CODE_25GBASE_CR_CA_S 0x0C
#define PM_QSFP_EXT_COMPLIANCE_CODE_25GBASE_CR_CA_N 0x0D
#define PM_QSFP_EXT_COMPLIANCE_CODE_40GBASE_ER4     0x10
#define PM_QSFP_EXT_COMPLIANCE_CODE_4x10GBASE_SR    0x11
#define PM_QSFP_EXT_COMPLIANCE_CODE_40GBASE_PSM4    0x12
#define PM_QSFP_EXT_COMPLIANCE_CODE_100GBASE_CLR4   0x17

//      QSFP Diagnostic Monitoring Type Structure
typedef struct {
        /* byte 220 */
        unsigned char   reserved_220_0:1;
        unsigned char   reserved_220_1:1;
        unsigned char   reserved_220_2:1;
        unsigned char   average_input_optical_power:1; // Power Measurement
        unsigned char   reserved_220_4:1;
        unsigned char   reserved_220_5:1;
        unsigned char   reserved_220_6:1;
        unsigned char   reserved_220_7:1;
} pm_qsfp_diag_monitoring_type_t;

//      QSFP Enhanced Options Structure
typedef struct {
        /* byte 221 */
        unsigned char   reserved_221_0:1;
        unsigned char   reserved_221_1:1;
        unsigned char   application_rate_selection:1; // Appl. Select Table
        unsigned char   rate_selection_supported:1; // Extended Rate Selection
        unsigned char   reserved_221_4:1;
        unsigned char   reserved_221_5:1;
        unsigned char   reserved_221_6:1;
        unsigned char   reserved_221_7:1;
} pm_qsfp_enhanced_options_t;

//      Various field lengths in Page 0

typedef struct {
        // Base ID fields (64 bytes, 128 to 191)
        unsigned char   identifier;             // Type of transceiver
        pm_qsfp_ext_indentifier_t ext_identifier; // Extended ID
        unsigned char   connector;              // Connector type
        pm_qsfp_spec_compliance_t spec_compliance; // I/F compliance
        unsigned char   encoding;               // Serial Encoding
        unsigned char   bit_rate_nominal;       // in units of 100Mb/s
        pm_qsfp_rate_select_t rate_select;  // Rate Select Complianc
        unsigned char   length_smf_in_km;       // SM fiber, units: km
        unsigned char   length_om3_in_2m;       // OM3 fiber, units: 2m
        unsigned char   length_om2_in_m;        // OM2 fiber, units: m
        unsigned char   length_om1_in_m;        // OM1 fiber, units: m
        unsigned char   length_copper_in_m;     // Copper, units: m
        pm_qsfp_device_tech_t device_tech;  // Device Technology
        unsigned char   vendor_name[PM_VENDOR_NAME_LEN];
        pm_qsfp_ib_compliance_t ib_compliance; // IB Compliance Code
        unsigned char   vendor_oui[PM_VENDOR_OUI_LEN]; // IEEE ID
        unsigned char   vendor_part_number[PM_VENDOR_PN_LEN];
        unsigned char   vendor_revision[PM_QSFP_VENDOR_REV_LEN];
        pm_qsfp_wavelength_t wavelength;    // Laser WL/Copper Atten
        unsigned short  wavelength_tolerance;   // in 0.005nm
        unsigned char   max_case_temperature;   // in degrees C
        unsigned char   check_code_for_base;    // Sum of bytes 128-190
        pm_qsfp_options_t options;          // Options Implemented
        unsigned char   vendor_serial_number[PM_VENDOR_SN_LEN];
        pm_date_code_t vendor_date_code;    // MFG Date Code
        pm_qsfp_diag_monitoring_type_t diag_monitor_type; //
        pm_qsfp_enhanced_options_t enhanced_options; //
        unsigned char   reserved_222;
        unsigned char   check_code_for_extended; // Sum of bytes 192-222
        unsigned char vendor_area[PM_SERIAL_ID_VENDOR_AREA_LEN];
} pm_qsfp_serial_id_t;

#endif
