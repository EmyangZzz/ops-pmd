# -*- coding: utf-8 -*-
# (C) Copyright 2016 Hewlett Packard Enterprise Development LP
# All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.
#
##########################################################################

"""
OpenSwitch Test to verify the DOM information parameters range.
"""

from dom_supporting_xvrs import dom_xvrs


TOPOLOGY = """
# +-------+
# |  ops1 |
# +-------+

# Nodes
[type=openswitch name="OpenSwitch 1"] ops1
"""


dutarray = []

xvrs = dom_xvrs()


def check_dom_support(step, xvr_mod):

    if xvr_mod in xvrs:
        step("%s module in supported list" % (xvr_mod))
        return True

    return False


def verify_dom_info(step, intf):
    switch = dutarray[0]

    step("Verifying DOM info on interface: %s" % (intf))
    cmd = "show interface " + intf + " dom"
    bufferout = switch(cmd)
    bufferoutlist = bufferout.split('\n')

    for line in bufferoutlist:
        if "Temperature:" in line:
            str1 = line.split(" ")[-1]
            str2 = str1.split("C")[0]
            assert (-10.00 < float(str2) < 100.00), step(
                "### Failed in verifying temperature range ###")

        if "Voltage:" in line:
            str1 = line.split(" ")[-1]
            str2 = str1.split("V")[0]
            assert (2.50 < float(str2) < 3.80), step(
                "### Failed in verifying voltage range ###")


def check_error_msg(step, intf):
    switch = dutarray[0]

    step("Checking for error msg on interface: %s" % (intf))
    cmd = "show interface " + intf + " dom"
    bufferout = switch(cmd)
    bufferoutlist = bufferout.split('\n')

    if " % No DOM information available" not in bufferoutlist:
        step("### Failed in verifying DOM error output ###")


def verify_dom(step):
    switch = dutarray[0]

    # Verify to display transceiver DOM information through CLI
    step("\n\n######## Test to display transceiver DOM "
         "information through CLI ########")
    bufferout = switch("show interface transceiver")
    bufferoutlist = bufferout.split('\n')

    i = 0
    while(i < len(bufferoutlist)):
        if "Interface" in bufferoutlist[i]:
            intf = bufferoutlist[i].split(" ")[-1].strip()
            intf = intf[:-1]
            step("Interface: %s" % (intf))
            if '-' not in intf:
                _line = bufferoutlist[i + 2]
                if "Transceiver module" == _line.split(": ")[0].strip():
                    xvr_mod = _line.split(": ")[-1].strip()
                    step("Transceiver: %s" % (xvr_mod))
                    if(xvr_mod == "not present"):
                        step("Transceiver is not present")
                        i = i + 1
                        continue
                    else:
                        step("Transceiver is present")
                        if check_dom_support(step, xvr_mod):
                            verify_dom_info(step, intf)
                        else:
                            check_error_msg(step, intf)
        i = i + 1

    step("\n\n######## Successfully tested display transceiver DOM "
         "information through CLI ########")


def test_ft_dom2(topology, step):
    ops1 = topology.get("ops1")
    assert ops1 is not None

    global dutarray
    dutarray = [ops1]

    # Check DOM information for correctness using CLI
    verify_dom(step)
