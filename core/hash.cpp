#include <boost/crc.hpp>
#include <cstddef>
#include <cstdint>
uint32_t crc32(const void* data, std::size_t size) {
    // process_bytes потребує const void* data
    boost::crc_32_type crc;
    crc.process_bytes(data, size);
    return crc.checksum();
}
