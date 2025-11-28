// Copyright (c) 2025 Ziqi Fan
// SPDX-License-Identifier: Apache-2.0

//! ROS2 Message Type Definitions
//!
//! CDR serialization using serde + cdr crate
//!
//! Supports all ROS2 message types:
//!   - Basic types: bool, i8/u8, i16/u16, i32/u32, i64/u64, f32, f64
//!   - Strings: String
//!   - Fixed arrays: [T; N]
//!   - Dynamic arrays: Vec<T>
//!   - Nested structures: Direct composition
//!
//! Examples:
//!
//! // Basic types
//! #[derive(Serialize, Deserialize)]
//! struct BasicTypes {
//!     flag: bool,
//!     count: i32,
//!     value: f64,
//! }
//!
//! // String
//! #[derive(Serialize, Deserialize)]
//! struct StringMsg {
//!     name: String,
//!     description: String,
//! }
//!
//! // Fixed array
//! #[derive(Serialize, Deserialize)]
//! struct FixedArrayMsg {
//!     data: [f32; 10],
//!     xyz: [f64; 3],
//! }
//!
//! // Dynamic array
//! #[derive(Serialize, Deserialize)]
//! struct DynamicArrayMsg {
//!     values: Vec<i32>,
//!     names: Vec<String>,
//! }
//!
//! // Nested structure
//! #[derive(Serialize, Deserialize)]
//! struct Point { x: f64, y: f64, z: f64 }
//!
//! #[derive(Serialize, Deserialize)]
//! struct Pose {
//!     position: Point,
//!     orientation: Point,
//! }
//!
//! // Complex message with all features
//! #[derive(Serialize, Deserialize)]
//! struct ComplexMsg {
//!     id: i32,
//!     name: String,
//!     matrix: [[f32; 3]; 3],
//!     trajectory: Vec<Point>,
//! }

#![allow(dead_code)]

use serde::{Deserialize, Serialize};

// geometry_msgs/msg/Vector3
#[derive(Serialize, Deserialize, Debug, Clone, Default)]
pub struct Vector3 {
    pub x: f64,
    pub y: f64,
    pub z: f64,
}

// geometry_msgs/msg/Twist
#[derive(Serialize, Deserialize, Debug, Clone, Default)]
pub struct Twist {
    pub linear: Vector3,
    pub angular: Vector3,
}
