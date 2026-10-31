#include "core/txrx.h"
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <random>
#include <algorithm>
#include <termios.h>
#include <unistd.h>
#include <chrono>

std::vector<uint8_t> serialize(const p& data, size_t payload_size) {
    if (data.payload.size() != payload_size) {
        throw std::invalid_argument("Payload size does not match g_payload_size");
    }

    const size_t payload_bytes = payload_size * sizeof(int);
    const size_t total_size = sizeof(data.crc) +
                              sizeof(data.timestamp) +
                              sizeof(data.s_number) +
                              payload_bytes;

    std::vector<uint8_t> buffer(total_size);
    size_t offset = 0;

    auto write_field = [&](const void* src, size_t size) {
        std::memcpy(buffer.data() + offset, src, size);
        offset += size;
    };

    write_field(&data.crc, sizeof(data.crc));
    write_field(&data.timestamp, sizeof(data.timestamp));
    write_field(&data.s_number, sizeof(data.s_number));

    if (payload_bytes > 0) {
        write_field(data.payload.data(), payload_bytes);
    }

    return buffer;
}

p deserialize(const uint8_t* buffer, size_t buffer_size, size_t payload_size) {
    const size_t payload_bytes = payload_size * sizeof(int);
    const size_t expected_size = sizeof(uint32_t) +
                                 sizeof(int64_t) +
                                 sizeof(long) +
                                 payload_bytes;

    if (buffer_size < expected_size) {
        throw std::runtime_error("Buffer is too small for deserialization");
    }

    p result;
    size_t offset = 0;

    auto read_field = [&](void* dst, size_t size) {
        std::memcpy(dst, buffer + offset, size);
        offset += size;
    };

    read_field(&result.crc, sizeof(result.crc));
    read_field(&result.timestamp, sizeof(result.timestamp));
    read_field(&result.s_number, sizeof(result.s_number));

    result.payload.resize(payload_size);
    if (payload_bytes > 0) {
        read_field(result.payload.data(), payload_bytes);
    }

    return result;
}

void fill_vector(std::vector<int>& vec, int minVal, int maxVal) {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<int> dist(minVal, maxVal);

    std::generate(vec.begin(), vec.end(), [&]() {
        return dist(gen);
    });
}

int getch_unix() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Вимикаємо буферизацію та відображення
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    int ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Повертаємо налаштування
    return ch;
}

int64_t getTimestampSeconds() {
    return std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::system_clock::now().time_since_epoch()
               ).count();
}
