#include "ihdr.h"
#include <algorithm>

void IHDR::read(std::vector<uint8_t> data) {

    std::memcpy(reinterpret_cast<char *>(this), data.data(), sizeof(IHDR));
    std::reverse(reinterpret_cast<char *>(&width), reinterpret_cast<char *>(&width) + sizeof(uint32_t));
    std::reverse(reinterpret_cast<char *>(&height), reinterpret_cast<char *>(&height) + sizeof(uint32_t));
    validate();
}

bool is_valid_combination(uint8_t color_type, uint8_t bit_depth) {
    std::vector<uint32_t> allowed_bit_depths = {0b11111, 0, 0b1100, 0b1111, 0b1100, 0, 0b1100};
    return allowed_bit_depths[color_type] & bit_depth;
}

void IHDR::validate() const {
    uint32_t max_len = static_cast<uint32_t>(1) << 31;
    if (width == 0 || width > max_len) {
        throw IHDRException("width must be in [1; 2^31], but given: " + std::to_string(width));
    }
    if (height == 0 || height > max_len) {
        throw IHDRException("height must be in [1; 2^31], but given: " + std::to_string(width));
    }
    if (bit_depth == 0 || (bit_depth & (bit_depth - 1)) != 0 || bit_depth > 16) {
        throw IHDRException("bit_depth = " + std::to_string(bit_depth) + " != 1, 2, 4, 8 or 16");
    }
    if (color_type != 0 && color_type != 2 && color_type != 3 && color_type != 4 && color_type != 6) {
        throw IHDRException("color_type = " + std::to_string(color_type) + " != 0, 2, 3, 4 or 6");
    }

    if (!is_valid_combination(color_type, bit_depth)) {
        throw IHDRException(
                "prohibited combination of color type: " + std::to_string(color_type) + " and bit depth: " +
                std::to_string(bit_depth));
    }

    if (compression_method != 0) {
        throw IHDRException("compression_method = " + std::to_string(compression_method) + " != 0");
    }
    if (filter_method != 0) {
        throw IHDRException("filter_method = " + std::to_string(filter_method) + " != 0");
    }
    if (interlace_method != 0 && interlace_method != 1) {
        throw IHDRException("interlace_method = " + std::to_string(interlace_method) + " != 0 or 1");
    }
}

std::size_t IHDR::bytes_per_pixel() const {
    int bytes_pp = bit_depth / 8;
    if (color_type == 2) {
        bytes_pp *= 3;
    } else if (color_type == 6) {
        bytes_pp *= 4;
    } else if (color_type == 4) {
        bytes_pp *= 2;
    } else {
        bytes_pp = 1;
    }
    return bytes_pp;
}