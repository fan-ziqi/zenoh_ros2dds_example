// Copyright (c) 2025 Ziqi Fan
// SPDX-License-Identifier: Apache-2.0

#pragma once
/**
 * Single-header CDR serialization
 *
 * Supports all ROS2 message types
 * (https://docs.ros.org/en/jazzy/Concepts/Basic/About-Interfaces.html):
 *   - Basic types: bool, byte, char, int8~64, uint8~64, float32, float64
 *   - Strings: string (std::string), wstring (std::u16string)
 *   - Fixed arrays: Type[N], std::array<T,N>
 *   - Dynamic arrays: sequence<Type> (std::vector<Type>)
 *   - Nested structures: Direct composition
 *
 * Usage:
 *   struct MyMsg { float a; std::string name; };
 *   auto data = cdr::serialize(msg);
 *   cdr::deserialize(data, len, msg);
 */

#include <array>
#include <boost/pfr.hpp>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace cdr {

// ==================== CDR Writer ====================

class Writer {
   public:
    Writer() { buffer_.reserve(256); }

    // Basic types (ROS2: bool, int8, uint8, int16, uint16, int32, uint32, int64, uint64, float32,
    // float64)
    Writer& operator<<(bool v) {
        uint8_t b = v ? 1 : 0;
        write(&b, 1);
        return *this;
    }
    Writer& operator<<(int8_t v) {
        write(&v, 1);
        return *this;
    }
    Writer& operator<<(uint8_t v) {
        write(&v, 1);
        return *this;
    }
    Writer& operator<<(int16_t v) {
        align(2);
        write(&v, 2);
        return *this;
    }
    Writer& operator<<(uint16_t v) {
        align(2);
        write(&v, 2);
        return *this;
    }
    Writer& operator<<(int32_t v) {
        align(4);
        write(&v, 4);
        return *this;
    }
    Writer& operator<<(uint32_t v) {
        align(4);
        write(&v, 4);
        return *this;
    }
    Writer& operator<<(int64_t v) {
        align(8);
        write(&v, 8);
        return *this;
    }
    Writer& operator<<(uint64_t v) {
        align(8);
        write(&v, 8);
        return *this;
    }
    Writer& operator<<(float v) {
        align(4);
        write(&v, 4);
        return *this;
    }
    Writer& operator<<(double v) {
        align(8);
        write(&v, 8);
        return *this;
    }

    // char (ROS2: char = uint8)
    Writer& operator<<(char v) {
        write(&v, 1);
        return *this;
    }

    // string (ROS2: string)
    Writer& operator<<(const std::string& s) {
        *this << static_cast<uint32_t>(s.size() + 1);  // Including null terminator
        write(s.data(), s.size());
        uint8_t null = 0;
        write(&null, 1);
        return *this;
    }

    // C string
    Writer& operator<<(const char* s) { return *this << std::string(s); }

    // wstring (ROS2: wstring, CDR: 4-byte length + UTF-16LE)
    Writer& operator<<(const std::u16string& s) {
        *this << static_cast<uint32_t>(s.size() + 1);  // Including null terminator
        for (char16_t c : s) {
            align(2);
            write(&c, 2);
        }
        char16_t null = 0;
        align(2);
        write(&null, 2);
        return *this;
    }

    // Fixed array T[N] (ROS2: Type[N])
    template <typename T, size_t N>
    Writer& operator<<(const T (&arr)[N]) {
        for (size_t i = 0; i < N; i++) *this << arr[i];
        return *this;
    }

    // std::array<T, N>
    template <typename T, size_t N>
    Writer& operator<<(const std::array<T, N>& arr) {
        for (size_t i = 0; i < N; i++) *this << arr[i];
        return *this;
    }

    // Dynamic array std::vector<T> (ROS2: sequence<Type>)
    template <typename T>
    Writer& operator<<(const std::vector<T>& vec) {
        *this << static_cast<uint32_t>(vec.size());
        for (const auto& v : vec) *this << v;
        return *this;
    }

    // Aggregate type (struct reflection)
    template <typename T>
    auto operator<<(const T& obj) -> std::enable_if_t<std::is_aggregate_v<T>, Writer&> {
        boost::pfr::for_each_field(obj, [this](const auto& field) { *this << field; });
        return *this;
    }

    std::vector<uint8_t> finish() {
        std::vector<uint8_t> result;
        result.reserve(4 + buffer_.size());
        // CDR header (little endian)
        result.push_back(0x00);
        result.push_back(0x01);
        result.push_back(0x00);
        result.push_back(0x00);
        result.insert(result.end(), buffer_.begin(), buffer_.end());
        return result;
    }

   private:
    void align(size_t n) {
        size_t pad = (n - (buffer_.size() % n)) % n;
        for (size_t i = 0; i < pad; i++) buffer_.push_back(0);
    }
    void write(const void* data, size_t len) {
        const uint8_t* p = static_cast<const uint8_t*>(data);
        buffer_.insert(buffer_.end(), p, p + len);
    }
    std::vector<uint8_t> buffer_;
};

// ==================== CDR Reader ====================

class Reader {
   public:
    Reader(const uint8_t* data, size_t len)
        : data_(data + 4), len_(len > 4 ? len - 4 : 0), pos_(0), ok_(len >= 4) {}

    bool ok() const { return ok_; }

    // Basic types
    Reader& operator>>(bool& v) {
        uint8_t b;
        read(&b, 1);
        v = (b != 0);
        return *this;
    }
    Reader& operator>>(int8_t& v) {
        read(&v, 1);
        return *this;
    }
    Reader& operator>>(uint8_t& v) {
        read(&v, 1);
        return *this;
    }
    Reader& operator>>(int16_t& v) {
        align(2);
        read(&v, 2);
        return *this;
    }
    Reader& operator>>(uint16_t& v) {
        align(2);
        read(&v, 2);
        return *this;
    }
    Reader& operator>>(int32_t& v) {
        align(4);
        read(&v, 4);
        return *this;
    }
    Reader& operator>>(uint32_t& v) {
        align(4);
        read(&v, 4);
        return *this;
    }
    Reader& operator>>(int64_t& v) {
        align(8);
        read(&v, 8);
        return *this;
    }
    Reader& operator>>(uint64_t& v) {
        align(8);
        read(&v, 8);
        return *this;
    }
    Reader& operator>>(float& v) {
        align(4);
        read(&v, 4);
        return *this;
    }
    Reader& operator>>(double& v) {
        align(8);
        read(&v, 8);
        return *this;
    }

    // char
    Reader& operator>>(char& v) {
        read(&v, 1);
        return *this;
    }

    // string
    Reader& operator>>(std::string& s) {
        uint32_t len;
        *this >> len;
        if (ok_ && len > 0 && len < 1000000) {
            s.resize(len - 1);  // Excluding null terminator
            read(s.data(), len - 1);
            pos_++;  // Skip null terminator
        }
        return *this;
    }

    // wstring
    Reader& operator>>(std::u16string& s) {
        uint32_t len;
        *this >> len;
        if (ok_ && len > 0 && len < 1000000) {
            s.resize(len - 1);
            for (size_t i = 0; i < len - 1; i++) {
                align(2);
                read(&s[i], 2);
            }
            align(2);
            pos_ += 2;  // Skip null terminator
        }
        return *this;
    }

    // Fixed array T[N]
    template <typename T, size_t N>
    Reader& operator>>(T (&arr)[N]) {
        for (size_t i = 0; i < N; i++) *this >> arr[i];
        return *this;
    }

    // std::array<T, N>
    template <typename T, size_t N>
    Reader& operator>>(std::array<T, N>& arr) {
        for (size_t i = 0; i < N; i++) *this >> arr[i];
        return *this;
    }

    // Dynamic array std::vector<T>
    template <typename T>
    Reader& operator>>(std::vector<T>& vec) {
        uint32_t size;
        *this >> size;
        if (ok_ && size < 1000000) {
            vec.resize(size);
            for (auto& v : vec) *this >> v;
        }
        return *this;
    }

    // Aggregate type (struct reflection)
    template <typename T>
    auto operator>>(T& obj) -> std::enable_if_t<std::is_aggregate_v<T>, Reader&> {
        boost::pfr::for_each_field(obj, [this](auto& field) { *this >> field; });
        return *this;
    }

   private:
    void align(size_t n) { pos_ += (n - (pos_ % n)) % n; }
    void read(void* out, size_t len) {
        if (pos_ + len <= len_) {
            memcpy(out, data_ + pos_, len);
            pos_ += len;
        } else {
            ok_ = false;
        }
    }
    const uint8_t* data_;
    size_t len_, pos_;
    bool ok_;
};

// ==================== Convenience Functions ====================

template <typename T>
std::vector<uint8_t> serialize(const T& obj) {
    Writer w;
    w << obj;
    return w.finish();
}

template <typename T>
bool deserialize(const uint8_t* data, size_t len, T& obj) {
    Reader r(data, len);
    r >> obj;
    return r.ok();
}

}  // namespace cdr
