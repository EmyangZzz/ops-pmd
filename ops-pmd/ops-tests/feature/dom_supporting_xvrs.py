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
DOM information supporting transceivers.
"""

xvrs = [
    "SFP_SX",
    "SFP_LX",
    "SFP_SR",
    "SFP_LR",
    "QSFP_LR4",
    "QSFP_SR4"
]


def dom_xvrs():
    return xvrs


__all__ = ['dom_xvrs']
