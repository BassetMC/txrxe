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

std::vector<uint8_t> serialize(const p& data, size_t payload_size);
p deserialize(const uint8_t* buffer, size_t buffer_size, size_t payload_size);
constexpr size_t get_header_size() noexcept;
constexpr size_t get_packet_size(size_t payload_size) noexcept;
void fill_vector(std::vector<int>& vec, int minVal, int maxVal);
int getch_unix();
int64_t getTimestampSeconds();