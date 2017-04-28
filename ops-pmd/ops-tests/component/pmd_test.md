# pmd Test Cases

## Test initial conditions
### Objective
Verify that the pm\_info in the interface has an initial status of connector=absent and connector\_status=unrecognized.
### Requirements
 - The Virtual Mininet test setup is required for this test.
### Setup
#### Topology diagram
```
[s1]
```
### Description ###
1. Get the list of interfaces.
2. Verify that the interfaces list is not empty.
3. For each interface:
  1. Verify that the pm\_info data is present.
  2. Verify that the pm\_info "connector" is "absent".
  3. Confirm that the pm\_info "connector\_status" is unrecognized.
### Test Result Criteria
#### Test Pass Criteria
All verifications succeed.
#### Test Fail Criteria
One or more verifications fail.

## Test insertion and removal of SFP modules
### Objective
Verify that the insertion and deletion is detected and EEPROM data is parsed.
### Requirements
 - The Virtual Mininet test setup is required for this test.
### Setup
#### Topology diagram
```
[s1]
```
### Description
1. Select a SFP interface.
2. Verify that the pm\_info "connector" is "absent" and "connector\_status" is "unrecognized".
3. For each SFP test data set:
  1. Simulate the module insertion.
  2. Verify that the data in pm\_info matches expected data.
  3. Simulate the module removal.
  4. Verify that the pm\_info "connector" is "absent" and "connector\_status" is "unrecognized".
  5. Verify that no other values are in pm\_info.

### Test result criteria
#### Test pass criteria
All verifications succeed.
#### Test fail criteria
One or more verifications fail.

## Test insertion and removal of QSFP modules
### Objective
Verify that the QSPF interface insertion and deletion is detected and EEPROM data is parsed.
### Requirements
The Virtual Mininet test setup is required for this test.
### Setup
#### Topology diagram
```
[s1]
```
### Description
1. Select a QSFP interface.
2. Verify that the pm\_info "connector" is "absent" and "connector\_status" is "unrecognized".
3. For each QSFP test data set.
  1. Simulate the module insertion.
  2. Verify that the data in pm\_info matches expected data.
  3. Simulate the module removal.
  4. Verify that the pm\_info "connector" is "absent" and "connector\_status" is "unrecognized".
  5. Verify that no other values are in pm\_info.
### Test result criteria
#### Test pass criteria
All verifications succeed.
#### Test fail criteria
One or more verifications fail.
