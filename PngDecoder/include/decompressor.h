#pragma once

#include <libdeflate.h>
#include <vector>
#include <iostream>

struct DeflateInsufficientSpace : std::runtime_error {
    explicit DeflateInsufficientSpace() : std::runtime_error("Deflating: insufficient space.\n") {};
};

struct DeflateDeflatingFailed : std::runtime_error {
    explicit DeflateDeflatingFailed() : std::runtime_error("Deflating: deflating failed.\n") {};
};

struct DeflateBadData : std::runtime_error {
    explicit DeflateBadData() : std::runtime_error("Deflating: provided bad data.\n") {};
};

struct DeflateShortOutput : std::runtime_error {
    explicit DeflateShortOutput() : std::runtime_error("Deflating: short output.\n") {};
};

struct PngDecompressor {
    PngDecompressor(const PngDecompressor &other) = delete;

    PngDecompressor(PngDecompressor
                    &&other) = delete;

    PngDecompressor &operator=(const PngDecompressor &other) = delete;

    PngDecompressor &operator=(PngDecompressor &&other) = delete;


    PngDecompressor() : decompressor(libdeflate_alloc_decompressor()) {}

    [[nodiscard]] std::vector<uint8_t> deflate(const std::vector<uint8_t> &data) const;

    ~PngDecompressor() {
        libdeflate_free_decompressor(decompressor);
    }

private:
    libdeflate_decompressor *decompressor;
};