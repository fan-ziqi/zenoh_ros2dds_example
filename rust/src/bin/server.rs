// Copyright (c) 2025 Ziqi Fan
// SPDX-License-Identifier: Apache-2.0

//! Zenoh Service Server
//! Implements ROS2 example_interfaces/srv/AddTwoInts

use std::env;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use zenoh::{Config, Wait};
use zenoh_ros2dds_example::cdr;
use zenoh_ros2dds_example::srv::{AddTwoIntsRequest, AddTwoIntsResponse};

fn main() {
    let args: Vec<String> = env::args().collect();

    let bridge_addr = if args.len() > 1 {
        args[1].clone()
    } else {
        eprintln!("Usage: {} <bridge_address>", args[0]);
        eprintln!("Example: {} localhost:7447", args[0]);
        return;
    };

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

    println!("Zenoh Service Server started");
    println!("  Connection: tcp/{}", bridge_addr);
    println!("  Service: add_two_ints (ROS2 /add_two_ints)");
    println!("  Waiting for requests... (Ctrl+C to exit)");

    // Declare Queryable
    let queryable = session
        .declare_queryable("add_two_ints")
        .callback(move |query| {
            println!(">> Received request: {}", query.key_expr());

            if let Some(payload) = query.payload() {
                let data = payload.to_bytes();
                match cdr::deserialize::<AddTwoIntsRequest>(&data) {
                    Ok(request) => {
                        println!("   Data: a={}, b={}", request.a, request.b);

                        // Build response
                        let response = AddTwoIntsResponse {
                            sum: request.a + request.b,
                        };

                        // Serialize
                        let response_data = cdr::serialize(&response).unwrap();

                        // Send response
                        if let Err(e) = query.reply(query.key_expr().clone(), response_data).wait()
                        {
                            eprintln!("   Failed to send response: {}", e);
                        } else {
                            println!("<< Sent response: sum={}", response.sum);
                        }
                    }
                    Err(e) => eprintln!("   Deserialization error: {}", e),
                }
            }
        })
        .wait()
        .expect("Failed to create service");

    // Wait for exit
    let running = Arc::new(AtomicBool::new(true));
    let r = running.clone();
    ctrlc::set_handler(move || {
        r.store(false, Ordering::SeqCst);
    })
    .ok();

    while running.load(Ordering::SeqCst) {
        std::thread::sleep(std::time::Duration::from_millis(100));
    }

    println!("\nStopping");
    drop(queryable);
    session.close().wait().ok();
}
