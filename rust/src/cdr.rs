// Copyright (c) 2025 Ziqi Fan
// SPDX-License-Identifier: Apache-2.0

//! CDR serialization wrapper

use serde::{Deserialize, Serialize};

/// Serialize a message to CDR format (Little Endian)
pub fn serialize<T: Serialize>(msg: &T) -> Result<Vec<u8>, cdr::Error> {
    ::cdr::serialize::<_, _, ::cdr::CdrLe>(msg, ::cdr::Infinite)
}

/// Deserialize a message from CDR format
pub fn deserialize<'a, T: Deserialize<'a>>(data: &'a [u8]) -> Result<T, cdr::Error> {
    ::cdr::deserialize(data)
}
