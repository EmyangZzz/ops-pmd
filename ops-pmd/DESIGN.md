# High level design of ops-pmd
ops-pmd is the platform daemon that processess and manages pluggable modules in the platform.

## Reponsibilities
ops-pmd is responsible for
* detecting pluggable module insertion and removal events
* reading the pluggable module information to determine type and characteristics
* saving pluggable module information into the database
* monitoring requests (via the database) to enable/disable the laser for fiber transceivers

## Design choices
N/A

## Relationships to external OpenSwitch entities
```ditaa
  +----------+     +----------+
  | ops-pmd  +---->+  OVSDB   |
  +----------+     +----------+
       |   |
       |   |                 +-------------------+
       |   +---------------->| pluggable modules |
       v                     +-------------------+
  +--------------------+
  |hw description files|
  +--------------------+
```
ops-pmd deals with three entities
* OVSDB - All pluggable module state and status is in the database
* hardware description files - Uses config-yaml library to access hardware information
* pluggable modules - using config-yaml i2c library to read the pluggable module device info

## OVSDB-Schema
The following cols are written by ops-pmd
```
  Interface:pm_info
            Pluggable module information
  daemon["ops-pmd"]:cur_hw
            ops-pmd sets to '1' when it has completed initializtion
```

The following cols are read by ops-pmd
```
  Interface:name
            Name of the interface
  Interface:hw_intf_config
            Hardware configuration information, used to know when to enable/disable a laser
  subsystem:name
            Name of the subsystem this Interface is a member of
```

## Internal structure

### Main loop
Main loop pseudo-code
```
  initialize OVS IDL
  initialize appctl interface
  while not exiting
  if db has been configured
     check for any changes in port table
     check for any changes in vlan table
     if any changes
        calculate new vlan states and update db
        enable/disable lasers for any that have changed
  check for appctl
  wait for IDL or appctl input
```


### Source modules
```ditaa
  +-------+
  | pmd.c |                            +---------------------+
  |       |       +----------+         | config_yaml library |    +----------------------+
  |       +------>+ config.c +--------^+                     +--->+ hw description files |
  |       |       +----------+         |                     |    +----------------------+
  |       |                            |                     |
  |       |       +----------+         |            +--------+
  |       +------>+  plug.c  +--------------------> | i2c    |    +----------------------+
  |       |       +----+-----+         |            |        +--->+ pluggable modules    |
  |       |            |               +------------+--------+    +----------------------+
  |       |            v
  |       |    +---------------+
  |       |    | pmd_detect.c  |
  |       |    +---------------+
  |       |
  |       |
  |       |     +--------------+        +-------+
  |       +---->+ ovs_access.c +------->+ OVSDB |
  |       |     +--------------+        +-------+
  +-------+
```

### Data structures

#### Transceiver information
```
pm_sfp_serial_id_t: 64 bytes (0-63) of basic transceiver information, including...
   pm_sfp_serial_id_transceiver_t: Bytes 3-10 of the transceiver information.
   pm_sfp_options_t: Transceiver extended options, btyes 0-1
   pm_data_code_t: Contains the pluggable module manufacturing date information.
   pm_sfp_diag_monitoring_type_t: 0xA2 diagnostic information
   pm_sfp_enhanced_options_t: Transciever enhanced options information

pm_qsfp_serial_id_t: QSFP transceiver info
   pm_qsfp_ext_indentifier_t: QSFP extended identifier bytes (byte 129)
   pm_qsfp_spec_compliance_t: QSFP specification compliance information (bytes 131-138)
   pm_qsfp_rate_select_t: QSFP extended rate select (byte 141)
   pm_qsfp_device_tech_t: QSFP status (byte 147)
   pm_qsfp_ib_compliance_t: Infiniband compliance codes (byte 164)
   pm_qsfp_wavelength_t: laser wavelength/copper attenuation
   pm_qsfp_options_t: QSFP extended options (bytes 192-195)
   pm_qsfp_diag_monitoring_type_t: QSFP diag info (byte 220)
   pm_qsfp_enhanced_options_t: QSFP enhanced options (btye 221)
```

#### OpenSwitch database information
```
ovs_module_info: transceiver module information that is pushed into the OpenSwitch database
```

#### Internal port information
```
pm_port_t: Internal structure storing port information
```

## References
* [ops-config-yaml](/documents/dev/ops-config-yaml/DESIGN)
* [ops-hw-desc-files](/documents/dev/ops-hw-config/DESIGN)
