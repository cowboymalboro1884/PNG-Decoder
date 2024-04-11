#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include "filter.h"
#include "ihdr.h"
#include "decompressor.h"
#include "image.h"
#include "image_iterable.h"

struct PNGException : std::runtime_error {
    explicit PNGException(const std::string &msg) : std::runtime_error("PNG Decoding error: " + msg + ".\n") {};
};

struct CouldNotOpenFile : std::runtime_error {
    explicit CouldNotOpenFile(const std::string &filename) : std::runtime_error(
            "Could not open file: " + filename + ".\n") {};
};

struct InvalidFileFormat : std::runtime_error {
    explicit InvalidFileFormat() : std::runtime_error(
            "Could not process current file: invalid file format.\n") {};
};

struct InvalidPNGFormat : std::runtime_error {
    explicit InvalidPNGFormat() : std::runtime_error(
            "Could not process current png: invalid format.\n") {};

};

struct Chunk {
    uint32_t length;
    char type[4];
    std::vector<uint8_t> data;
    uint32_t crc;
};

class PngDecoder {
    std::istream &_is;
    IHDR _ihdr{};
    std::vector<Chunk> _chunks;
    std::vector<RGB> _palette;
    PngDecompressor _decompressor;

    void validate_header() const;

    void read() {
        read_chunks();
        read_ihdr();
    }

    void read_ihdr();

    void fill_palette();

    void read_chunks();

    void replace_data(std::vector<uint8_t> &data) const;

    std::vector<uint8_t> decompress();

    [[nodiscard]] Image get_image(std::vector<uint8_t> data) const;

    [[nodiscard]] uint8_t read_ith_pixel_from_byte(uint8_t byte, size_t i) const;

    [[nodiscard]] RGB get_pixel_with_offset(const std::vector<uint8_t> &data, int offset) const;

    [[nodiscard]] Image get_image_by_offset(std::vector<uint8_t> data, int &offset, size_t height, size_t width) const;

    [[nodiscard]] Image concatenate_interlaced_images(std::vector<ImageIterable> images) const;

public:

    explicit PngDecoder(std::istream &istream_) : _is(istream_) {}

    Image process() {
        validate_header();
        read();

        return get_image(decompress());
    }
};
