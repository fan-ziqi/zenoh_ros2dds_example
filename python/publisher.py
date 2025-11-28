#!/usr/bin/env python3
# Copyright (c) 2025 Ziqi Fan
# SPDX-License-Identifier: Apache-2.0

"""
Publish cmd_vel messages to ROS2 using Zenoh
"""

import argparse
import time

import zenoh

from msg import Twist, Vector3


def main():
    parser = argparse.ArgumentParser(description="Zenoh cmd_vel publisher")
    parser.add_argument(
        "--bridge",
        "-b",
        type=str,
        required=True,
        help="Bridge address, e.g.: localhost:7447",
    )
    parser.add_argument(
        "--linear",
        "-l",
        type=float,
        default=0.5,
        help="Linear velocity (default: %(default)s)",
    )
    parser.add_argument(
        "--angular",
        "-a",
        type=float,
        default=0.2,
        help="Angular velocity (default: %(default)s)",
    )
    args = parser.parse_args()

    config = zenoh.Config()
    config.insert_json5("connect/endpoints", f'["tcp/{args.bridge}"]')

    session = zenoh.open(config)
    publisher = session.declare_publisher("cmd_vel")

    print(f"Zenoh cmd_vel publisher started")  # noqa: F541
    print(f"  Connection: tcp/{args.bridge}")
    print(f"  Topic: cmd_vel -> ROS2 /cmd_vel")  # noqa: F541
    print(f"  Velocity: linear.x={args.linear}, angular.z={args.angular}")
    print()

    try:
        while True:
            twist = Twist(
                linear=Vector3(x=args.linear, y=0.0, z=0.0),
                angular=Vector3(x=0.0, y=0.0, z=args.angular),
            )

            publisher.put(twist.serialize())
            print(f"Published: linear.x={args.linear}, angular.z={args.angular}")
            time.sleep(1.0)

    except KeyboardInterrupt:
        print("\nStopping")
    finally:
        session.close()


if __name__ == "__main__":
    main()
