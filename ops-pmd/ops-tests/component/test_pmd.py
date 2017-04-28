# -*- coding: utf-8 -*-

# (c) Copyright 2015 Hewlett Packard Enterprise Development LP
#
# GNU Zebra is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any
# later version.
#
# GNU Zebra is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Zebra; see the file COPYING.  If not, write to the Free
# Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.

import time
from pytest import fixture
from os.path import dirname, isdir
from os import chdir
from json import loads
from shutil import copy

TOPOLOGY = """
# +-------+
# |  sw1  |
# +-------+

# Nodes
[type=openswitch name="Switch 1"] sw1
"""


test_file_dir = "/files"
sfp_interface = "21"
qsfp_interface = "49"
# sample files and expected results for SFPs
sfp_files = {
    "SFP_DAC_MOLEX.bin": {
        "cable_length": "1",
        "cable_technology": "passive",
        "connector": "SFP_DAC",
        "connector_status": "supported",
        "max_speed": "10000",
        "supported_speeds": "10000",
        "vendor_name": "Molex Inc.",
        "vendor_oui": "00-09-3a",
        "vendor_part_number": "747649124",
        "vendor_revision": "A1",
        "vendor_serial_number": "302330039"
    },
    "SFP_SR_AVAGO.bin": {
        "connector": "SFP_SR",
        "connector_status": "supported",
        "max_speed": "10000",
        "supported_speeds": "10000",
        "vendor_name": "AVAGO",
        "vendor_oui": "00-17-6a",
        "vendor_part_number": "AFBR-703SDZ-HP1",
        "vendor_revision": "G2.3",
        "vendor_serial_number": "AA0938A0DZ2"
    },
    "SFP_SR_AVAGO_CORRUPTED.bin": {
        "connector": "unknown",
        "connector_status": "unrecognized",
    }
}
# sample files and expected results for QSFPs
qsfp_files = {
    "QSFP_CR4_MELLANOX.bin": {
        "connector": "QSFP_CR4",
        "connector_status": "supported",
        "max_speed": "40000",
        "supported_speeds": "40000",
        "vendor_name": "Mellanox",
        "vendor_oui": "00-02-c9",
        "vendor_part_number": "670759-B22",
        "vendor_revision": "A1",
        "vendor_serial_number": "6C222903CM"
    },
    "QSFP_CR4_MOLEX.bin": {
        "connector": "QSFP_CR4",
        "connector_status": "supported",
        "max_speed": "40000",
        "supported_speeds": "40000",
        "vendor_name": "Molex Inc.",
        "vendor_oui": "00-09-3a",
        "vendor_part_number": "1110409083",
        "vendor_revision": "A0",
        "vendor_serial_number": "211730113"
    },
    "QSFP_SR4_AVAGO.bin": {
        "connector": "QSFP_SR4",
        "connector_status": "supported",
        "max_speed": "40000",
        "supported_speeds": "40000",
        "vendor_name": "AVAGO",
        "vendor_oui": "00-17-6a",
        "vendor_part_number": "AFBR-79EEPZ-HP1",
        "vendor_revision": "01",
        "vendor_serial_number": "ATA114110000012"
    }
}


# Create an automatic fixture to perform a chdir into the test directory.
# Required in order to use data files from the local directory.
@fixture(scope="module", autouse=True)
def datadir(request):
    filename = request.module.__file__
    _dirname = dirname(filename)
    _dirname = _dirname + test_file_dir
    if isdir(_dirname):
        chdir(_dirname)
    return _dirname


def insert_pluggable(interface, module, sw1):
    copy(module, sw1.shared_dir)
    sw1("ovs-appctl -t ops-pmd ops-pmd/sim {} insert /tmp/{}"
        "".format(interface, module), shell='bash')
    time.sleep(0.5)


def remove_pluggable(interface, sw1):
    sw1("ovs-appctl -t ops-pmd ops-pmd/sim {} remove"
        "".format(interface), shell='bash')
    time.sleep(0.5)


def get_interface(interface, sw1):
    pm_info = dict()
    out = sw1("ovs-vsctl --columns=pm_info --format=json list interface {}"
              "".format(interface), shell='bash')
    raw = loads(out)
    data = raw["data"][0][0][1]
    for item in data:
        pm_info[item[0]] = item[1]
    return pm_info


def get_interfaces(sw1):
    result = list()
    out = sw1("ovs-vsctl --columns=name list interface", shell='bash')
    lines = out.split("\n")
    for line in lines:
        if line == '\r' or line == "" or line == "\n":
            continue
        name_val = line.split(": ")
        name = name_val[0].strip()
        val = name_val[1].strip()
        if name == "name":
            result.append(val)
    return result


def _test_insert_remove_module(interface, dataset, sw1):
    pm_info = get_interface(interface, sw1)
    assert pm_info is not None
    assert pm_info["connector"] == "absent"
    assert pm_info["connector_status"] == "unrecognized"
    for module in dataset:
        insert_pluggable(interface, module, sw1)
        pm_info = get_interface(interface, sw1)
        reference_info = dataset[module]
        for attribute in reference_info:
            assert reference_info[attribute] == pm_info[attribute]
        remove_pluggable(interface, sw1)
        pm_info = get_interface(interface, sw1)
        assert pm_info["connector"] == "absent"
        assert pm_info["connector_status"] == "unrecognized"


def test_pmd(topology, step):
    sw1 = topology.get("sw1")
    step("1-Testing initial conditions\n")
    interfaces = get_interfaces(sw1)
    assert len(interfaces) is not 0
    for interface in interfaces:
        interface = interface.replace('"', '')
        # FIXME: Should not filter the sentence below
        if interface == "bridge_normal":
            continue
        pm_info = get_interface(interface, sw1)
        # some interfaces are really sub-interfaces - don't process those
        if len(pm_info) is 0:
            continue
        if pm_info != "None" and pm_info is not None:
            assert pm_info["connector"] == "absent"
            assert pm_info["connector_status"] == "unrecognized"
            assert len(pm_info) == 2
    step("2-Testing module insertion/removal of SFP+s\n")
    _test_insert_remove_module(sfp_interface, sfp_files, sw1)
    step("3-Testing module insertion/removal of QSFP+s\n")
    _test_insert_remove_module(qsfp_interface, qsfp_files, sw1)
