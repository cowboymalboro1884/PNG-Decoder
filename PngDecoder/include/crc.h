#pragma once

#include <vector>

static const unsigned BITS = 32;
static const unsigned TRUNC_POLY = 0x4C11DB7;
static const unsigned INIT_REM = 0xFFFFFFFF;
static const unsigned FINAL_XOR = 0xFFFFFFFF;
static const unsigned REFLECT_IN = true;
static const unsigned REFLECT_REM = true;

struct PngCrcData {
    const char *data;
    std::size_t len;
};

uint32_t calculate_crc(const std::vector<PngCrcData> &chunks);