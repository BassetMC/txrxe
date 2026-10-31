#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>

struct p {
    uint32_t crc;
    int64_t timestamp;
    long s_number;
    std::vector<int> payload;
};

constexpr size_t get_header_size() noexcept {
    return sizeof(p::crc) + sizeof(p::timestamp) + sizeof(p::s_number);
}

constexpr size_t get_packet_size(size_t payload_size) noexcept {
    return get_header_size() + payload_size * sizeof(decltype(p::payload)::value_type);
}

std::vector<uint8_t> serialize(const p& data, size_t payload_size);
p deserialize(const uint8_t* buffer, size_t buffer_size, size_t payload_size);
void fill_vector(std::vector<int>& vec, int minVal, int maxVal);
int getch_unix();
int64_t getTimestampSeconds();
bool write_all(int fd, const uint8_t* data, size_t size);
bool read_all(int fd, uint8_t* buffer, size_t size);
