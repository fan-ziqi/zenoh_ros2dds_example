// Copyright (c) 2025 Ziqi Fan
// SPDX-License-Identifier: Apache-2.0

#pragma once
/**
 * ROS2 Service Type Definitions
 */

#include "cdr.hpp"

namespace srv {

// ==================== example_interfaces ====================

// example_interfaces/srv/AddTwoInts Request
struct AddTwoIntsRequest {
    int64_t a;
    int64_t b;
};

// example_interfaces/srv/AddTwoInts Response
struct AddTwoIntsResponse {
    int64_t sum;
};

// Convenient aliases
using cdr::deserialize;
using cdr::serialize;

}  // namespace srv
