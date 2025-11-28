// Copyright (c) 2025 Ziqi Fan
// SPDX-License-Identifier: Apache-2.0

#pragma once
/**
 * ROS2 Message Type Definitions
 *
 * Supports all ROS2 message types:
 *   - Basic types: bool, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t,
 *                  int64_t, uint64_t, float, double
 *   - Strings: std::string
 *   - Fixed arrays: T[N] or std::array<T, N>
 *   - Dynamic arrays: std::vector<T>
 *   - Nested structures: Direct composition
 *
 * Examples:
 *
 * // Basic types
 * struct BasicTypes {
 *     bool flag;
 *     int32_t count;
 *     double value;
 * };
 *
 * // String
 * struct StringMsg {
 *     std::string name;
 *     std::string description;
 * };
 *
 * // Fixed array
 * struct FixedArrayMsg {
 *     float data[10];              // C-style array
 *     std::array<double, 3> xyz;   // std::array
 * };
 *
 * // Dynamic array
 * struct DynamicArrayMsg {
 *     std::vector<int32_t> values;
 *     std::vector<std::string> names;
 * };
 *
 * // Nested structure
 * struct Point { double x, y, z; };
 * struct Pose {
 *     Point position;
 *     Point orientation;
 * };
 *
 * // Complex message with all features
 * struct ComplexMsg {
 *     int32_t id;
 *     std::string name;
 *     float matrix[3][3];
 *     std::vector<Point> trajectory;
 * };
 */

#include "cdr.hpp"

namespace msg {

// geometry_msgs/msg/Vector3
struct Vector3 {
    double x;
    double y;
    double z;
};

// geometry_msgs/msg/Twist
struct Twist {
    Vector3 linear;
    Vector3 angular;
};

using cdr::deserialize;
using cdr::serialize;

}  // namespace msg
