
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <nmmintrin.h> // Містить інтринсіки SSE4.2 (або можна <immintrin.h>)
/**
 * Апаратний підрахунок CRC32-C (SSE4.2)
 * Пропускна здатність: 15–25+ ГБ/с на одне процесорне ядро.
 */
 inline uint32_t crc32c_hardware(const void* data, size_t size, uint32_t initial_crc = 0xFFFFFFFF) {
    uint64_t crc = initial_crc;
    const uint8_t* ptr = static_cast<const uint8_t*>(data);
    // 1. Основний цикл: обробка великими блоками по 8 байтів (64 біти)
    while (size >= sizeof(uint64_t)) {
        uint64_t val;
        // std::memcpy гарантує безпеку щодо вирівнювання (strict aliasing);
        // компілятор з -O3 згортає це в звичайну інструкцію movq без виклику функції
        std::memcpy(&val, ptr, sizeof(val));

        crc = _mm_crc32_u64(crc, val);

        ptr += sizeof(uint64_t);
        size -= sizeof(uint64_t);
    }
    // 2. Обробка залишку (хвіст менше 8 байтів)
    uint32_t crc32_val = static_cast<uint32_t>(crc);
    if (size >= sizeof(uint32_t)) {
        uint32_t val;
        std::memcpy(&val, ptr, sizeof(val));
        crc32_val = _mm_crc32_u32(crc32_val, val);
        ptr += sizeof(uint32_t);
        size -= sizeof(uint32_t);
    }
    if (size >= sizeof(uint16_t)) {
        uint16_t val;
        std::memcpy(&val, ptr, sizeof(val));
        crc32_val = _mm_crc32_u16(crc32_val, val);
        ptr += sizeof(uint16_t);
        size -= sizeof(uint16_t);
    }
    if (size == 1) {
        crc32_val = _mm_crc32_u8(crc32_val, *ptr);
    }
    // Фінальний XOR (стандарт для більшості реалізацій CRC32)
    return crc32_val ^ 0xFFFFFFFF;
}



uint32_t crc32(const void* data, std::size_t size) {
    return crc32c_hardware(data, size);
}



