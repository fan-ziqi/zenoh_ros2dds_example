# Copyright (c) 2025 Ziqi Fan
# SPDX-License-Identifier: Apache-2.0

"""
ROS2 Service Type Definitions (using pycdr2)
"""

from dataclasses import dataclass

from pycdr2 import IdlStruct


# example_interfaces/srv/AddTwoInts Request
@dataclass
class AddTwoIntsRequest(IdlStruct):
    a: int
    b: int


# example_interfaces/srv/AddTwoInts Response
@dataclass
class AddTwoIntsResponse(IdlStruct):
    sum: int
