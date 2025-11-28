// Copyright (c) 2025 Ziqi Fan
// SPDX-License-Identifier: Apache-2.0

#include <zenoh.h>

#include <cstring>
#include <iostream>

#include "srv.hpp"

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " <bridge_address>" << std::endl;
    std::cout << "Example: " << prog << " localhost:7447" << std::endl;
}

void query_handler(z_loaned_query_t* query, void* ctx) {
    const z_loaned_keyexpr_t* keyexpr = z_query_keyexpr(query);
    z_view_string_t keystr;
    z_keyexpr_as_view_string(keyexpr, &keystr);
    std::cout << ">> Received request: " << z_string_data(z_loan(keystr)) << std::endl;

    // Get payload
    const z_loaned_bytes_t* payload_bytes = z_query_payload(query);
    if (payload_bytes == nullptr) {
        std::cerr << "   Payload is empty" << std::endl;
        return;
    }

    // Read payload
    z_bytes_reader_t reader = z_bytes_get_reader(payload_bytes);
    std::vector<uint8_t> payload_data;
    uint8_t buffer[1024];

    while (true) {
        size_t read_len = z_bytes_reader_read(&reader, buffer, sizeof(buffer));
        if (read_len == 0) break;
        payload_data.insert(payload_data.end(), buffer, buffer + read_len);
    }

    if (payload_data.empty()) {
        std::cerr << "   Payload data is empty" << std::endl;
        return;
    }

    srv::AddTwoIntsRequest request;
    if (srv::deserialize(payload_data.data(), payload_data.size(), request)) {
        std::cout << "   Data: a=" << request.a << ", b=" << request.b << std::endl;

        // Build response
        srv::AddTwoIntsResponse response;
        response.sum = request.a + request.b;

        // Serialize
        auto response_data = srv::serialize(response);

        // Send response
        z_query_reply_options_t options;
        z_query_reply_options_default(&options);

        z_owned_bytes_t reply_payload;
        z_bytes_from_buf(&reply_payload, response_data.data(), response_data.size(), NULL, NULL);

        z_query_reply(query, keyexpr, z_move(reply_payload), &options);
        std::cout << "<< Sent response: sum=" << response.sum << std::endl;
    } else {
        std::cerr << "   Deserialization failed" << std::endl;
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

    std::cout << "Zenoh Service Server started" << std::endl;
    std::cout << "  Connection: tcp/" << bridge_addr << std::endl;
    std::cout << "  Service: add_two_ints (ROS2 /add_two_ints)" << std::endl;
    std::cout << "  Waiting for requests... (Ctrl+C to exit)" << std::endl;

    z_view_keyexpr_t keyexpr;
    z_view_keyexpr_from_str(&keyexpr, "add_two_ints");

    z_owned_closure_query_t closure;
    z_closure_query(&closure, query_handler, NULL, NULL);

    z_owned_queryable_t queryable;
    if (z_declare_queryable(z_loan(session), &queryable, z_loan(keyexpr), z_move(closure), NULL) <
        0) {
        std::cerr << "Failed to create service" << std::endl;
        z_drop(z_move(session));
        return 1;
    }

    while (true) {
        z_sleep_s(1);
    }

    z_drop(z_move(queryable));
    z_drop(z_move(session));
    return 0;
}
