# Copyright (c) 2025 Ziqi Fan
# SPDX-License-Identifier: Apache-2.0

"""
ROS2 Message Type Definitions (using pycdr2)

Dependency: pip install pycdr2

Supports all ROS2 message types:
  - Basic types: bool, int8~64, uint8~64, float32, float64
  - Strings: str
  - Fixed arrays: array[T, N]
  - Dynamic arrays: sequence[T]
  - Nested structures: Direct composition

Examples:

# Basic types
@dataclass
class BasicTypes(IdlStruct):
    flag: bool
    count: int32
    value: float64

# String
@dataclass
class StringMsg(IdlStruct):
    name: str
    description: str

# Fixed array
@dataclass
class FixedArrayMsg(IdlStruct):
    data: array[float32, 10]
    xyz: array[float64, 3]

# Dynamic array
@dataclass
class DynamicArrayMsg(IdlStruct):
    values: sequence[int32]
    names: sequence[str]

# Nested structure
@dataclass
class Point(IdlStruct):
    x: float64
    y: float64
    z: float64

@dataclass
class Pose(IdlStruct):
    position: Point
    orientation: Point

# Complex message with all features
@dataclass
class ComplexMsg(IdlStruct):
    id: int32
    name: str
    matrix: array[array[float32, 3], 3]
    trajectory: sequence[Point]
"""

from dataclasses import dataclass

from pycdr2 import IdlStruct
from pycdr2.types import (  # noqa: F401
    array,
    float32,
    float64,
    int8,
    int16,
    int32,
    int64,
    sequence,
    uint8,
    uint16,
    uint32,
    uint64,
)


# geometry_msgs/msg/Vector3
@dataclass
class Vector3(IdlStruct):
    x: float64
    y: float64
    z: float64


# geometry_msgs/msg/Twist
@dataclass
class Twist(IdlStruct):
    linear: Vector3
    angular: Vector3
