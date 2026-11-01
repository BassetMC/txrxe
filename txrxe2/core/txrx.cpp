#include "core/txrx.h"
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <random>
#include <termios.h>
#include <unistd.h>
#include <chrono>

p::p(size_t payload_size) {
    this->crc = 0;
    this->s_number = 0;
    this->timestamp = 0;
    this->payload.resize(payload_size);
}

void serialize(const p& data, size_t payload_size, std::vector<uint8_t> &buffer) {
    if (data.payload.size() != payload_size) {
        throw std::invalid_argument("Payload size does not match g_payload_size");
    }

    const size_t payload_bytes = payload_size * sizeof(int);
    const size_t total_size = sizeof(data.crc) +
                              sizeof(data.timestamp) +
                              sizeof(data.s_number) +
                              payload_bytes;

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

}

void deserialize(const uint8_t* buffer, size_t buffer_size, size_t payload_size, p& result) {
    const size_t payload_bytes = payload_size * sizeof(int);
    const size_t expected_size = sizeof(uint32_t) +
                                 sizeof(int64_t) +
                                 sizeof(long) +
                                 payload_bytes;

    if (buffer_size < expected_size) {
        throw std::runtime_error("Buffer is too small for deserialization");
    }


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

}

void fill_vector(std::vector<int>& vec, int minVal, int maxVal) {
    // 8 незалежних ліній стану Xorshift128+ (рівно під 256-бітні регістри AVX2)
    struct alignas(32) SimdRngState {
        uint64_t s0[8];
        uint64_t s1[8];

        SimdRngState() {
            std::random_device rd;
            for (int i = 0; i < 8; ++i) {
                s0[i] = (static_cast<uint64_t>(rd()) << 32) | rd();
                s1[i] = (static_cast<uint64_t>(rd()) << 32) | rd();
                if (s0[i] == 0 && s1[i] == 0) s0[i] = 1; // стан не має бути нульовим
            }
        }
    };

    // Ініціалізується один раз на потік
    static thread_local SimdRngState rng;

    const uint64_t range = static_cast<uint64_t>(static_cast<int64_t>(maxVal) - static_cast<int64_t>(minVal) + 1);
    int* __restrict dst = vec.data();
    size_t n = vec.size();

    size_t i = 0;
    // Основний SIMD-цикл: генерує по 8 випадкових чисел за ітерацію
    for (; i + 8 <= n; i += 8) {
        for (int k = 0; k < 8; ++k) {
            uint64_t x = rng.s0[k];
            const uint64_t y = rng.s1[k];
            rng.s0[k] = y;
            x ^= x << 23;
            uint64_t s1_new = x ^ y ^ (x >> 17) ^ (y >> 26);
            rng.s1[k] = s1_new;

            // 32 випадкові біти з 64-бітного стану
            uint32_t r = static_cast<uint32_t>((s1_new + y) >> 32);

            // Метод Леміра: рівномірне масштабування в [minVal, maxVal] через множення і зсув (без ділення!)
            dst[i + k] = minVal + static_cast<int>((static_cast<uint64_t>(r) * range) >> 32);
        }
    }

    // Обробка хвоста, якщо розмір вектора не кратний 8
    for (; i < n; ++i) {
        uint64_t x = rng.s0[0];
        const uint64_t y = rng.s1[0];
        rng.s0[0] = y;
        x ^= x << 23;
        rng.s1[0] = x ^ y ^ (x >> 17) ^ (y >> 26);
        uint32_t r = static_cast<uint32_t>((rng.s1[0] + y) >> 32);
        dst[i] = minVal + static_cast<int>((static_cast<uint64_t>(r) * range) >> 32);
    }
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

bool write_all(int fd, const uint8_t* data, size_t size) {
    size_t total_written = 0;
    while (total_written < size) {
        ssize_t n = write(fd, data + total_written, size - total_written);
        if (n > 0) {
            total_written += n;
        } else if (n == -1 && errno == EINTR) {
            continue; // Якщо виклик перервано сигналом
        } else {
            return false; // Помилка або канал закрито
        }
    }
    return true;
}
bool read_all(int fd, uint8_t* buffer, size_t size) {
    size_t total_read = 0;
    while (total_read < size) {
        ssize_t n = read(fd, buffer + total_read, size - total_read);
        if (n > 0) {
            total_read += n;
        } else if (n == 0) {
            return false; // EOF: Producer закрив канал
        } else if (errno == EINTR) {
            continue; // Якщо виклик перервано сигналом
        } else {
            return false; // Помилка читання
        }
    }
    return true;
}