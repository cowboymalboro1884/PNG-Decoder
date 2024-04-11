#pragma once

#include <cinttypes>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>

struct IHDRException : std::runtime_error {
    explicit IHDRException(const std::string &msg) : std::runtime_error("Invalid IHDR: " + msg + ".\n") {};
};

#pragma pack(push, 1)

struct IHDR {
    uint32_t width;
    uint32_t height;
    uint8_t bit_depth;
    uint8_t color_type;
    uint8_t compression_method;
    uint8_t filter_method;
    uint8_t interlace_method;

    void read(std::vector<uint8_t> data);

    void validate() const;

    [[nodiscard]] std::size_t bytes_per_pixel() const;
};

#pragma pack(pop)