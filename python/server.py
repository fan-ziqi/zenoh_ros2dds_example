#!/usr/bin/env python3
# Copyright (c) 2025 Ziqi Fan
# SPDX-License-Identifier: Apache-2.0

"""
Zenoh Service Server Example
Simulates ROS2 example_interfaces/srv/AddTwoInts service for ROS2 Client to call
"""

import argparse
import time

import zenoh

from srv import AddTwoIntsRequest, AddTwoIntsResponse


def main():
    parser = argparse.ArgumentParser(description="Zenoh AddTwoInts Service Server")
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

    print(f"Zenoh Service Server started")  # noqa: F541
    print(f"  Connection: tcp/{args.bridge}")
    print(f"  Service: add_two_ints (ROS2 /add_two_ints)")  # noqa: F541
    print("  Waiting for requests... (Ctrl+C to exit)")

    def query_handler(query):
        print(f">> Received request: {query.key_expr}")

        # 1. Get request data
        payload = query.payload
        if payload is None:
            print("   Error: Empty payload")
            return

        try:
            # 2. Deserialize request (CDR)
            request = AddTwoIntsRequest.deserialize(payload.to_bytes())
            print(f"   Data: a={request.a}, b={request.b}")

            # 3. Process logic
            result = request.a + request.b

            # 4. Build response
            response = AddTwoIntsResponse(sum=result)
            response_data = response.serialize()

            # 5. Send response
            query.reply(query.key_expr, response_data)
            print(f"<< Sent response: sum={result}")

        except Exception as e:
            print(f"   Processing error: {e}")

    # Declare Queryable (corresponds to ROS2 Service Server)
    queryable = session.declare_queryable("add_two_ints", query_handler)

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nStopping")
    finally:
        queryable.undeclare()
        session.close()


if __name__ == "__main__":
    main()
