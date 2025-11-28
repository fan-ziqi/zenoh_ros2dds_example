// Copyright (c) 2025 Ziqi Fan
// SPDX-License-Identifier: Apache-2.0

#include <zenoh.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "msg.hpp"

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " <bridge_address> [linear_x] [angular_z]" << std::endl;
    std::cout << "Example: " << prog << " localhost:7447 0.5 0.2" << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Error: Bridge address must be specified" << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    const char* bridge_addr = argv[1];
    double linear_x = (argc > 2) ? std::atof(argv[2]) : 0.5;
    double angular_z = (argc > 3) ? std::atof(argv[3]) : 0.2;

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

    z_owned_publisher_t publisher;
    z_view_keyexpr_t keyexpr;
    z_view_keyexpr_from_str(&keyexpr, "cmd_vel");

    if (z_declare_publisher(z_loan(session), &publisher, z_loan(keyexpr), NULL) < 0) {
        std::cerr << "Failed to create publisher" << std::endl;
        z_drop(z_move(session));
        return 1;
    }

    std::cout << "Zenoh cmd_vel publisher started" << std::endl;
    std::cout << "  Connection: tcp/" << bridge_addr << std::endl;
    std::cout << "  Topic: cmd_vel -> ROS2 /cmd_vel" << std::endl;
    std::cout << "  Velocity: linear.x=" << linear_x << ", angular.z=" << angular_z << std::endl;
    std::cout << std::endl;

    while (true) {
        msg::Twist twist{{linear_x, 0, 0}, {0, 0, angular_z}};
        auto payload = msg::serialize(twist);

        z_owned_bytes_t data;
        z_bytes_from_buf(&data, payload.data(), payload.size(), NULL, NULL);
        z_publisher_put(z_loan(publisher), z_move(data), NULL);

        std::cout << "Published: linear.x=" << linear_x << ", angular.z=" << angular_z << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    z_drop(z_move(publisher));
    z_drop(z_move(session));
    return 0;
}
