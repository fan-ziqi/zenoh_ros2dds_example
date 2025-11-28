// Copyright (c) 2025 Ziqi Fan
// SPDX-License-Identifier: Apache-2.0

use std::env;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use zenoh::{Config, Wait};
use zenoh_ros2dds_example::cdr;
use zenoh_ros2dds_example::msg::Twist;

fn main() {
    let args: Vec<String> = env::args().collect();

    let bridge_addr = if args.len() > 1 {
        args[1].clone()
    } else {
        eprintln!("Error: Bridge address must be specified");
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

    println!("Zenoh cmd_vel subscriber started");
    println!("  Connection: tcp/{}", bridge_addr);
    println!("  Topic: cmd_vel (ROS2 /cmd_vel)");
    println!();

    let subscriber = session
        .declare_subscriber("cmd_vel")
        .callback(|sample| {
            let data = sample.payload().to_bytes();
            match cdr::deserialize::<Twist>(&data) {
                Ok(twist) => {
                    println!(
                        "Received: linear.x={:.3}, angular.z={:.3}",
                        twist.linear.x, twist.angular.z
                    );
                }
                Err(e) => {
                    eprintln!("Deserialization error: {}", e);
                }
            }
        })
        .wait()
        .expect("Failed to create subscriber");

    println!("Waiting for messages... (Ctrl+C to exit)");

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
    drop(subscriber);
    session.close().wait().expect("Failed to close session");
}
