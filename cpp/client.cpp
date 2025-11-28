// Copyright (c) 2025 Ziqi Fan
// SPDX-License-Identifier: Apache-2.0

#include <zenoh.h>

#include <cstring>
#include <iostream>

#include "srv.hpp"

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " <bridge_address> [a] [b]" << std::endl;
    std::cout << "Example: " << prog << " localhost:7447 3 5" << std::endl;
}

void reply_handler(const z_loaned_reply_t* reply, void* ctx) {
    if (z_reply_is_ok(reply)) {
        const z_loaned_sample_t* sample = z_reply_ok(reply);
        z_view_slice_t payload;
        z_bytes_get_contiguous_view(z_sample_payload(sample), &payload);

        const uint8_t* data = reinterpret_cast<const uint8_t*>(z_slice_data(z_loan(payload)));
        size_t len = z_slice_len(z_loan(payload));

        srv::AddTwoIntsResponse response;
        if (srv::deserialize(data, len, response)) {
            std::cout << "Received response: sum=" << response.sum << std::endl;
        } else {
            std::cerr << "Deserialization failed" << std::endl;
        }
    } else {
        std::cerr << "Service call failed" << std::endl;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Error: Bridge address must be specified" << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    const char* bridge_addr = argv[1];
    int64_t a = (argc > 2) ? std::atoll(argv[2]) : 3;
    int64_t b = (argc > 3) ? std::atoll(argv[3]) : 5;

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

    std::cout << "Zenoh Service Client started" << std::endl;
    std::cout << "  Connection: tcp/" << bridge_addr << std::endl;
    std::cout << "  Service: add_two_ints (ROS2 /add_two_ints)" << std::endl;
    std::cout << std::endl;

    // Build request
    srv::AddTwoIntsRequest request{a, b};
    auto request_data = srv::serialize(request);

    std::cout << "Sending request: a=" << a << ", b=" << b << std::endl;

    // Send query
    z_view_keyexpr_t keyexpr;
    z_view_keyexpr_from_str(&keyexpr, "add_two_ints");

    z_owned_bytes_t payload;
    z_bytes_from_buf(&payload, request_data.data(), request_data.size(), NULL, NULL);

    z_get_options_t opts;
    z_get_options_default(&opts);
    opts.timeout_ms = 5000;
    opts.payload = z_move(payload);

    z_owned_fifo_handler_reply_t handler;
    z_owned_closure_reply_t closure;
    z_fifo_channel_reply_new(&closure, &handler, 16);

    z_get(z_loan(session), z_loan(keyexpr), "", z_move(closure), &opts);

    z_owned_reply_t reply;
    while (z_recv(z_loan(handler), &reply) == Z_OK) {
        reply_handler(z_loan(reply), NULL);
        z_drop(z_move(reply));
    }

    z_drop(z_move(handler));
    z_drop(z_move(session));
    return 0;
}
