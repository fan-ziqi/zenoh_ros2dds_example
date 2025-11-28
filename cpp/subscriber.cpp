// Copyright (c) 2025 Ziqi Fan
// SPDX-License-Identifier: Apache-2.0

#include <zenoh.h>

#include <iostream>

#include "msg.hpp"

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " <bridge_address>" << std::endl;
    std::cout << "Example: " << prog << " localhost:7447" << std::endl;
}

void callback(z_loaned_sample_t* sample, void* arg) {
    z_view_slice_t payload;
    z_bytes_get_contiguous_view(z_sample_payload(sample), &payload);

    const uint8_t* data = reinterpret_cast<const uint8_t*>(z_slice_data(z_loan(payload)));
    size_t len = z_slice_len(z_loan(payload));

    msg::Twist twist;
    if (msg::deserialize(data, len, twist)) {
        std::cout << "Received: linear.x=" << twist.linear.x << ", angular.z=" << twist.angular.z
                  << std::endl;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Error: Bridge address must be specified" << std::endl;
        print_usage(argv[0]);
        return 1;
    }
    const char* bridge_addr = argv[1];

    z_owned_config_t config;
    z_config_default(&config);

    char endpoint[256];
    snprintf(endpoint, sizeof(endpoint), "[\"tcp/%s\"]", bridge_addr);

    if (zc_config_insert_json5(z_loan_mut(config), Z_CONFIG_CONNECT_KEY, endpoint) < 0) {
        std::cerr << "Configuration error" << std::endl;
        return 1;
    }

    z_owned_session_t session;
    if (z_open(&session, z_move(config), NULL) < 0) {
        std::cerr << "Connection failed: " << bridge_addr << std::endl;
        return 1;
    }

    std::cout << "Zenoh cmd_vel subscriber started" << std::endl;
    std::cout << "  Connection: tcp/" << bridge_addr << std::endl;
    std::cout << "  Topic: cmd_vel (ROS2 /cmd_vel)" << std::endl;
    std::cout << std::endl;

    z_owned_closure_sample_t closure;
    z_closure_sample(&closure, callback, NULL, NULL);

    z_view_keyexpr_t keyexpr;
    z_view_keyexpr_from_str(&keyexpr, "cmd_vel");

    z_owned_subscriber_t subscriber;
    if (z_declare_subscriber(z_loan(session), &subscriber, z_loan(keyexpr), z_move(closure), NULL) <
        0) {
        std::cerr << "Failed to create subscriber" << std::endl;
        z_drop(z_move(session));
        return 1;
    }

    std::cout << "Waiting for messages... (Ctrl+C to exit)" << std::endl;

    while (true) {
        z_sleep_s(1);
    }

    z_drop(z_move(subscriber));
    z_drop(z_move(session));
    return 0;
}
