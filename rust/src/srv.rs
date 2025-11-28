// Copyright (c) 2025 Ziqi Fan
// SPDX-License-Identifier: Apache-2.0

//! ROS2 Service Type Definitions

#![allow(dead_code)]

use serde::{Deserialize, Serialize};

// ==================== example_interfaces ====================

/// example_interfaces/srv/AddTwoInts Request
#[derive(Serialize, Deserialize, Debug, Clone, Default)]
pub struct AddTwoIntsRequest {
    pub a: i64,
    pub b: i64,
}

/// example_interfaces/srv/AddTwoInts Response
#[derive(Serialize, Deserialize, Debug, Clone, Default)]
pub struct AddTwoIntsResponse {
    pub sum: i64,
}
