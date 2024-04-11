#include <boost/crc.hpp>
#include "crc.h"

uint32_t calculate_crc(const std::vector<PngCrcData> &chunks) {
    boost::crc_optimal<BITS, TRUNC_POLY, INIT_REM, FINAL_XOR, REFLECT_IN, REFLECT_REM> crc;
    for (auto bytes: chunks) {
        crc.process_bytes(bytes.data, bytes.len);
    }
    return crc.checksum();
}