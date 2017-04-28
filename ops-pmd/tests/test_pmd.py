#!/usr/bin/python
#
#  (c) Copyright 2015 Hewlett Packard Enterprise Development LP
#
#  Licensed under the Apache License, Version 2.0 (the "License"); you may
#  not use this file except in compliance with the License. You may obtain
#  a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#  License for the specific language governing permissions and limitations
#  under the License.
#

import os
import sys
import json
import time
import pytest
import subprocess
from opsvsi.docker import *
from opsvsi.opsvsitest import *

TEST_FILE_DIR = "/files"

SFP_INTERFACE = "21"
QSFP_INTERFACE = "49"

# sample files and expected results for SFPs
SFP_FILES = {
    "SFP_DAC_MOLEX.bin":{
        "cable_length":"1",
        "cable_technology":"passive",
        "connector":"SFP_DAC",
        "connector_status":"supported",
        "max_speed":"10000",
        "supported_speeds":"10000",
        "vendor_name":"Molex Inc.",
        "vendor_oui":"00-09-3a",
        "vendor_part_number":"747649124",
        "vendor_revision":"A1",
        "vendor_serial_number":"302330039"
    },
    "SFP_SR_AVAGO.bin":{
        "connector":"SFP_SR",
        "connector_status":"supported",
        "max_speed":"10000",
        "supported_speeds":"10000",
        "vendor_name":"AVAGO",
        "vendor_oui":"00-17-6a",
        "vendor_part_number":"AFBR-703SDZ-HP1",
        "vendor_revision":"G2.3",
        "vendor_serial_number":"AA0938A0DZ2"
    },
    "SFP_SR_AVAGO_CORRUPTED.bin":{
        "connector":"unknown",
        "connector_status":"unrecognized",
    }
}

# sample files and expected results for QSFPs
QSFP_FILES = {
    "QSFP_CR4_MELLANOX.bin":{
        "connector":"QSFP_CR4",
        "connector_status":"supported",
        "max_speed":"40000",
        "supported_speeds":"40000",
        "vendor_name":"Mellanox",
        "vendor_oui":"00-02-c9",
        "vendor_part_number":"670759-B22",
        "vendor_revision":"A1",
        "vendor_serial_number":"6C222903CM"
        },
    "QSFP_CR4_MOLEX.bin":{
        "connector":"QSFP_CR4",
        "connector_status":"supported",
        "max_speed":"40000",
        "supported_speeds":"40000",
        "vendor_name":"Molex Inc.",
        "vendor_oui":"00-09-3a",
        "vendor_part_number":"1110409083",
        "vendor_revision":"A0",
        "vendor_serial_number":"211730113"
        },
    "QSFP_SR4_AVAGO.bin":{
        "connector":"QSFP_SR4",
        "connector_status":"supported",
        "max_speed":"40000",
        "supported_speeds":"40000",
        "vendor_name":"AVAGO",
        "vendor_oui":"00-17-6a",
        "vendor_part_number":"AFBR-79EEPZ-HP1",
        "vendor_revision":"01",
        "vendor_serial_number":"ATA114110000012"
        }
}

# Create an automatic fixture to perform a chdir into the test directory.
# Required in order to use data files from the local directory.
@pytest.fixture(scope="module", autouse=True)
def datadir(request):
    filename = request.module.__file__
    dirname = os.path.dirname(filename)
    dirname = dirname + TEST_FILE_DIR

    if os.path.isdir(dirname):
        os.chdir(dirname)

    return dirname

# test class to perform operations on simulation engine
class pmdTest( OpsVsiTest ):

    def setupNet(self):
        # if you override this function, make sure to
        # either pass getNodeOpts() into hopts/sopts of the topology that
        # you build or into addHost/addSwitch calls
        topo=SingleSwitchTopo(
            k=1,
            hopts=self.getHostOpts(),
            sopts=self.getSwitchOpts())
        self.net = Mininet(topo=topo,
            switch=VsiOpenSwitch,
            host=Host,
            link=OpsVsiLink, controller=None,
            build=True)

    def insert_pluggable(self, interface, module):
        s1 = self.net.switches[0]
        shutil.copy(module, s1.shareddir)
        out = s1.cmd("/usr/bin/ovs-appctl -t ops-pmd ops-pmd/sim " + interface + \
                    " insert /shared/" + module)

    def remove_pluggable(self, interface):
        s1 = self.net.switches[0]
        out = s1.cmd("/usr/bin/ovs-appctl -t ops-pmd ops-pmd/sim " + interface + \
                    " remove")

    def get_interface(self, interface):
        s1 = self.net.switches[0]
        pm_info = dict()
        out = s1.cmd("/usr/bin/ovs-vsctl --columns=pm_info --format=json list interface " + interface)
        raw = json.loads(out)
        data = raw["data"][0][0][1]
        for item in data:
            pm_info[item[0]] = item[1]
        return pm_info

    def get_interfaces(self):
        s1 = self.net.switches[0]
        result = list()
        out = s1.ovscmd("/usr/bin/ovs-vsctl --columns=name list interface")
        lines = out.split("\n")
        for line in lines:
            if line == '\r' or line == "":
                continue
            name_val = line.split(":")
            name = name_val[0].strip()
            val = name_val[1].strip()
            if name == "name":
                result.append(val)
        return result

@pytest.mark.skipif(True, reason="Disabling old tests")
# class to execute tests
class Test_pmd:

    def setup(self):
        pass

    def teardown(self):
        pass

    def setup_class(cls):
        Test_pmd.test = pmdTest()

    def teardown_class(cls):
        # Stop the Docker containers, and
        # mininet topology
        Test_pmd.test.net.stop()

    def setup_method(self, method):
        pass

    def teardown_method(self, method):
        pass

    def __del__(self):
        del self.test

    # Interface daemon tests.
    def test_initial_conditions(self):
        info("Testing initial conditions\n")
        interfaces = self.test.get_interfaces()
        assert len(interfaces) != 0, "Expected interface count to be non-zero."
        for interface in interfaces:
            pm_info = self.test.get_interface(interface)
            # some interfaces are really sub-interfaces - don't process those
            if len(pm_info) == 0:
                continue
            if pm_info != "None":
                assert pm_info["connector"] == "absent", \
                        "Expected connector to be absent"
                assert pm_info["connector_status"] == "unrecognized", \
                        "Expected connector_status to be unrecognized"
                assert len(pm_info) == 2, "Unexpected data in pm_info"

    def _test_insert_remove_MODULE(self, interface, dataset):
        pm_info = self.test.get_interface(interface)
        assert pm_info != None, "Expected pm_info data"
        assert pm_info["connector"] == "absent", \
                "Expected connector to be absent"
        assert pm_info["connector_status"] == "unrecognized", \
                "Expected connector_status to be unrecognized"
        for module in dataset:
            self.test.insert_pluggable(interface, module)
            pm_info = self.test.get_interface(interface)
            reference_info = dataset[module]
            assert len(pm_info) == len(reference_info), \
                    "Expected reference data to match"
            for attribute in reference_info:
                assert reference_info[attribute] == pm_info[attribute], \
                        "Expected reference data to match"
            self.test.remove_pluggable(interface)
            pm_info = self.test.get_interface(interface)
            assert len(pm_info) == 2, "Unexpected data in pm_info"
            assert pm_info["connector"] == "absent", \
                    "Expected connector to be absent"
            assert pm_info["connector_status"] == "unrecognized", \
                    "Expected connector_status to be unrecognized"

    def test_insert_remove_SFP(self):
        info("Testing module insertion/removal of SFP+s\n")
        self._test_insert_remove_MODULE(SFP_INTERFACE, SFP_FILES)

    def test_insert_remove_QSFP(self):
        info("Testing module insertion/removal of QSFP+s\n")
        self._test_insert_remove_MODULE(QSFP_INTERFACE, QSFP_FILES)
