#!/usr/bin/env python3
# Copyright (c) 2025 Ziqi Fan
# SPDX-License-Identifier: Apache-2.0

"""
Zenoh Service Client Example
Calls ROS2 example_interfaces/srv/AddTwoInts service
"""

import argparse

import zenoh

from srv import AddTwoIntsRequest, AddTwoIntsResponse


def main():
    parser = argparse.ArgumentParser(description="Zenoh AddTwoInts Service Client")
    parser.add_argument(
        "--bridge",
        "-b",
        type=str,
        required=True,
        help="Bridge address, e.g.: localhost:7447",
    )
    parser.add_argument("--a", type=int, default=3, help="First number (default: 3)")
    parser.add_argument("--b", type=int, default=5, help="Second number (default: 5)")
    args = parser.parse_args()

    config = zenoh.Config()
    config.insert_json5("connect/endpoints", f'["tcp/{args.bridge}"]')

    session = zenoh.open(config)

    print(f"Zenoh Service Client started")  # noqa: F541
    print(f"  Connection: tcp/{args.bridge}")
    print(f"  Service: add_two_ints (ROS2 /add_two_ints)")  # noqa: F541
    print()

    # Build request
    request = AddTwoIntsRequest(a=args.a, b=args.b)
    request_data = request.serialize()

    print(f"Sending request: a={args.a}, b={args.b}")

    # Send query (Service Call)
    # In Zenoh, Service maps to key expression: <service_name>
    replies = session.get("add_two_ints", payload=request_data, timeout=5.0)

    # Handle response
    for reply in replies:
        if reply.ok:
            response_data = reply.ok.payload.to_bytes()
            try:
                response = AddTwoIntsResponse.deserialize(response_data)
                print(f"Received response: sum={response.sum}")
            except Exception as e:
                print(f"Deserialization error: {e}")
        else:
            print(f"Service call failed: {reply.err}")

    session.close()


if __name__ == "__main__":
    main()
