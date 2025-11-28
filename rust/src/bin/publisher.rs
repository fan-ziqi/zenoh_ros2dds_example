// Copyright (c) 2025 Ziqi Fan
// SPDX-License-Identifier: Apache-2.0

use std::env;
use std::thread;
use std::time::Duration;
use zenoh::{Config, Wait};
use zenoh_ros2dds_example::cdr;
use zenoh_ros2dds_example::msg::{Twist, Vector3};

fn main() {
    let args: Vec<String> = env::args().collect();

    let bridge_addr = if args.len() > 1 {
        args[1].clone()
    } else {
        eprintln!("Error: Bridge address must be specified");
        eprintln!("Usage: {} <bridge_address> [linear_x] [angular_z]", args[0]);
        eprintln!("Example: {} localhost:7447 0.5 0.2", args[0]);
        return;
    };

    let linear_x: f64 = args.get(2).and_then(|s| s.parse().ok()).unwrap_or(0.5);
    let angular_z: f64 = args.get(3).and_then(|s| s.parse().ok()).unwrap_or(0.2);

    let mut config = Config::default();
    config
        .insert_json5("connect/endpoints", &format!(r#"["tcp/{}"]"#, bridge_addr))
        .expect("Failed to configure connection endpoints");

    let session = match zenoh::open(config).wait() {
        Ok(s) => s,
        Err(e) => {
            eprintln!("Connection failed: {}", e);
            eprintln!("Usage: {} [bridge_address] [linear_x] [angular_z]", args[0]);
            eprintln!("Example: {} 192.168.1.100:7447 0.5 0.2", args[0]);
            return;
        }
    };

    let topic = "cmd_vel";
    let publisher = match session.declare_publisher(topic).wait() {
        Ok(p) => p,
        Err(e) => {
            eprintln!("Failed to create publisher: {}", e);
            return;
        }
    };

    println!("Zenoh cmd_vel publisher started");
    println!("  Connection: tcp/{}", bridge_addr);
    println!("  Topic: {} -> ROS2 /cmd_vel", topic);
    println!("  Velocity: linear.x={}, angular.z={}", linear_x, angular_z);
    println!();

    loop {
        let twist = Twist {
            linear: Vector3 {
                x: linear_x,
                y: 0.0,
                z: 0.0,
            },
            angular: Vector3 {
                x: 0.0,
                y: 0.0,
                z: angular_z,
            },
        };
        let payload = cdr::serialize(&twist).unwrap();

        if let Err(e) = publisher.put(payload).wait() {
            eprintln!("Publish error: {}", e);
        } else {
            println!("Published: linear.x={}, angular.z={}", linear_x, angular_z);
        }

        thread::sleep(Duration::from_secs(1));
    }
}
