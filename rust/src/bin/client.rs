// Copyright (c) 2025 Ziqi Fan
// SPDX-License-Identifier: Apache-2.0

//! Zenoh Service Client
//! Calls ROS2 example_interfaces/srv/AddTwoInts

use std::env;
use std::time::Duration;
use zenoh::{query::QueryTarget, Config, Wait};
use zenoh_ros2dds_example::cdr;
use zenoh_ros2dds_example::srv::{AddTwoIntsRequest, AddTwoIntsResponse};

fn main() {
    let args: Vec<String> = env::args().collect();

    let bridge_addr = if args.len() > 1 {
        args[1].clone()
    } else {
        eprintln!("Usage: {} <bridge_address> [a] [b]", args[0]);
        eprintln!("Example: {} localhost:7447 3 5", args[0]);
        return;
    };

    let a: i64 = args.get(2).and_then(|s| s.parse().ok()).unwrap_or(3);
    let b: i64 = args.get(3).and_then(|s| s.parse().ok()).unwrap_or(5);

    let mut config = Config::default();
    config
        .insert_json5("connect/endpoints", &format!(r#"["tcp/{}"]"#, bridge_addr))
        .expect("Failed to configure connection endpoints");

    let session = match zenoh::open(config).wait() {
        Ok(s) => s,
        Err(e) => {
            eprintln!("Connection failed: {}", e);
            return;
        }
    };

    println!("Zenoh Service Client started");
    println!("  Connection: tcp/{}", bridge_addr);
    println!("  Service: add_two_ints (ROS2 /add_two_ints)");
    println!();

    // Build request
    let request = AddTwoIntsRequest { a, b };

    // CDR serialization
    let request_data = cdr::serialize(&request).unwrap();

    println!("Sending request: a={}, b={}", a, b);

    // Send query (Service Call)
    let replies = session
        .get("add_two_ints")
        .payload(request_data)
        .timeout(Duration::from_secs(5))
        .target(QueryTarget::All)
        .wait()
        .expect("Query failed");

    // Handle response
    while let Ok(reply) = replies.recv() {
        match reply.result() {
            Ok(sample) => {
                let response_data = sample.payload().to_bytes();
                match cdr::deserialize::<AddTwoIntsResponse>(&response_data) {
                    Ok(response) => {
                        println!("Received response: sum={}", response.sum);
                    }
                    Err(e) => {
                        eprintln!("Deserialization error: {}", e);
                    }
                }
            }
            Err(e) => {
                eprintln!("Service call failed: {:?}", e);
            }
        }
    }

    session.close().wait().ok();
}
