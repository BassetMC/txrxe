#include <boost/crc.hpp>

uint32_t crc32(const void* data, std::size_t size) {
    // process_bytes потребує const void* data
    // можна було обійтися без boost но бібліотека на мою думку буде краще ніж функція з інтернету на константі
    // можливо потім порівняю різницю
    boost::crc_32_type crc;
    crc.process_bytes(data, size);
    return crc.checksum();
}
