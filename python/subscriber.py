#!/usr/bin/env python3
# Copyright (c) 2025 Ziqi Fan
# SPDX-License-Identifier: Apache-2.0

"""
Subscribe to ROS2 cmd_vel messages using Zenoh
"""

import argparse
import time

import zenoh

from msg import Twist


def main():
    parser = argparse.ArgumentParser(description="Zenoh cmd_vel subscriber")
    parser.add_argument(
        "--bridge",
        "-b",
        type=str,
        required=True,
        help="Bridge address, e.g.: localhost:7447",
    )
    args = parser.parse_args()

    config = zenoh.Config()
    config.insert_json5("connect/endpoints", f'["tcp/{args.bridge}"]')

    session = zenoh.open(config)

    print(f"Zenoh cmd_vel subscriber started")  # noqa: F541
    print(f"  Connection: tcp/{args.bridge}")
    print(f"  Topic: cmd_vel (ROS2 /cmd_vel)")  # noqa: F541
    print()

    def callback(sample):
        data = sample.payload.to_bytes()
        try:
            twist = Twist.deserialize(data)
            print(f"Received: linear.x={twist.linear.x:.3f}, angular.z={twist.angular.z:.3f}")
        except Exception as e:
            print(f"Parse error: {e}")

    subscriber = session.declare_subscriber("cmd_vel", callback)

    try:
        print("Waiting for messages... (Ctrl+C to exit)")
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nStopping")
    finally:
        subscriber.undeclare()
        session.close()


if __name__ == "__main__":
    main()
