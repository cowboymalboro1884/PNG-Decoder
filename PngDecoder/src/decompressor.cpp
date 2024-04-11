#include "decompressor.h"

std::vector<uint8_t> PngDecompressor::deflate(const std::vector<uint8_t> &data) const {
    std::vector<uint8_t> output(data.size() * 4);
    size_t actual_out;
    auto status = libdeflate_zlib_decompress(decompressor, data.data(), data.size(), output.data(), output.size(),
                                             &actual_out);
    if (status == libdeflate_result::LIBDEFLATE_SUCCESS) {
        output.resize(actual_out);
        return output;
    } else if (status == libdeflate_result::LIBDEFLATE_INSUFFICIENT_SPACE) {
        throw DeflateInsufficientSpace();
    } else if (status == libdeflate_result::LIBDEFLATE_BAD_DATA) {
        throw DeflateBadData();
    } else if (status == LIBDEFLATE_SHORT_OUTPUT) {
        throw DeflateShortOutput();
    } else {
        throw DeflateDeflatingFailed();
    }
}