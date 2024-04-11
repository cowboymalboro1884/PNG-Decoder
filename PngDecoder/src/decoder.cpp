#include "decoder.h"
#include "filter.h"
#include "bit_reader.h"
#include "crc.h"

void PngDecoder::validate_header() const {
    uint8_t expected[] = {137, 80, 78, 71, 13, 10, 26, 10};
    uint8_t actual[8];
    _is.read(reinterpret_cast<char *>(actual), 8);
    for (int i = 0; i < 8; i++) {
        if (expected[i] != actual[i]) {
            throw InvalidFileFormat();
        }
    }
}

void PngDecoder::read_ihdr() {
    std::vector<uint8_t> IHDR_data;
    IHDR_data = _chunks[0].data;
    if (IHDR_data.empty()) {
        throw InvalidPNGFormat();
    }
    _ihdr.read(IHDR_data);
}

void PngDecoder::fill_palette() {
    for (auto chunk: _chunks) {
        if (!std::memcmp("PLTE", chunk.type, sizeof(chunk.type))) {
            if (chunk.length % 3 != 0) {
                throw PNGException("invalid PLTE chunk, given length: " + std::to_string(chunk.length));
            }
            size_t cnt = chunk.length / 3;
            auto ptr_on_data = chunk.data.data();
            while (cnt--) {
                RGB p{};
                p.r = BitReader::read_bytes_reversed(reinterpret_cast<uint8_t *>(ptr_on_data));
                ptr_on_data += sizeof(uint8_t);
                p.g = BitReader::read_bytes_reversed(reinterpret_cast<uint8_t *>(ptr_on_data));
                ptr_on_data += sizeof(uint8_t);
                p.b = BitReader::read_bytes_reversed(reinterpret_cast<uint8_t *>(ptr_on_data));
                ptr_on_data += sizeof(uint8_t);

                _palette.push_back(p);
            }
        }
    }
}

void check_chunk_crc(const Chunk &chunk) {
    std::vector<PngCrcData> crc = {{chunk.type,                 sizeof(chunk.type)},
                                   {(char *) chunk.data.data(), chunk.length}};
    if (calculate_crc(crc) != chunk.crc) {
        throw PNGException("invalid CRC from " + std::string(chunk.type) + " chunk");
    }
}

void PngDecoder::read_chunks() {
    while (_is) {
        Chunk chunk;
        _is.read(reinterpret_cast<char *>(&chunk.length), 4);
        chunk.length = BitReader::reverse_bytes(chunk.length);

        _is.read(chunk.type, 4);
        if (!std::memcmp(chunk.type, "IEND", sizeof(chunk.type))) {
            break;
        }
        chunk.data.resize(chunk.length);
        _is.read(reinterpret_cast<char *>(chunk.data.data()), chunk.length);

        _is.read(reinterpret_cast<char *>(&chunk.crc), 4);
        chunk.crc = BitReader::reverse_bytes(chunk.crc);

        _chunks.push_back(chunk);
        if (_is.peek() == EOF) {
            break;
        }
    }

    for (const auto &chunk: _chunks) {
        check_chunk_crc(chunk);
    }

    fill_palette();
}

std::vector<uint8_t> PngDecoder::decompress() {
    std::vector<uint8_t> data;
    for (auto chunk: _chunks) {
        if (!std::memcmp(chunk.type, "IDAT", sizeof(chunk.type))) {
            data.insert(data.end(), chunk.data.begin(), chunk.data.end());
        }
    }
    if (data.empty()) {
        throw PNGException("could not find IDAT chunk");
    }
    return _decompressor.deflate(data);
}


Image PngDecoder::concatenate_interlaced_images(std::vector<ImageIterable> images) const {
    Image image = Image((int) _ihdr.height, (int) _ihdr.width);
    static const std::vector<std::vector<int>> pos = {{1, 6, 4, 6, 2, 6, 4, 6},
                                                      {3, 6, 4, 6, 3, 6, 4, 6}};

    for (int row = 0; row < image.Height(); row++) {
        for (int col = 0; col < image.Width(); col++) {
            if (row % 2 == 1) {
                image(row, col) = images[6].next();
                continue;
            }
            size_t offset_y = row % 8;
            size_t offset_x = col % 8;
            if (offset_x % 2 == 1) {
                image(row, col) = images[5].next();
            } else if (offset_y == 2 || offset_y == 6) {
                image(row, col) = images[4].next();
            } else if (offset_y == 0) {
                image(row, col) = images[pos[0][offset_x] - 1].next();
            } else {
                image(row, col) = images[pos[1][offset_x] - 1].next();
            }
        }
    }
    return image;
}

[[nodiscard]] Image PngDecoder::get_image(std::vector<uint8_t> data) const {
    int offset = 0;
    if (_ihdr.bit_depth < 8) {
        replace_data(data);
    }
    if (_ihdr.interlace_method == 0) {
        return get_image_by_offset(data, offset, _ihdr.height, _ihdr.width);
    }

    static const std::vector<std::pair<int, int>> layouts = {{7, 7},
                                                             {3, 7},
                                                             {3, 3},
                                                             {1, 3},
                                                             {0, 0},
                                                             {1, 1},
                                                             {0, 0}};
    static const std::vector<std::pair<int, int>> shape = {{8, 8},
                                                           {8, 8},
                                                           {4, 8},
                                                           {4, 4},
                                                           {2, 4},
                                                           {2, 2},
                                                           {1, 2}};

    std::vector<ImageIterable> interlaced_images;
    for (int i = 0; i < 7; ++i) {
        size_t width = (_ihdr.width + layouts[i].first) / shape[i].first;
        size_t height = (_ihdr.height + layouts[i].second) / shape[i].second;
        Image image = get_image_by_offset(data, offset, height, width);
        interlaced_images.emplace_back(image);
    }
    return concatenate_interlaced_images(interlaced_images);
}

uint8_t PngDecoder::read_ith_pixel_from_byte(uint8_t byte, size_t idx) const {
    uint8_t size = _ihdr.bit_depth;
    uint8_t mask;
    if (size == 1) {
        mask = 1;
    } else if (size == 2) {
        mask = 0b00000011;
    } else if (size == 4) {
        mask = 0b00001111;
    } else {
        throw PNGException("invalid bit depth: " + std::to_string(_ihdr.bit_depth));
    }
    return (byte >> (8 - idx - size)) & mask;
}

void PngDecoder::replace_data(std::vector<uint8_t> &data) const {
    int offset = 0;
    std::vector<uint8_t> new_data;
    for (size_t row = 0; row < _ihdr.height; row++) {
        uint8_t filter = data[offset];
        offset++;
        new_data.push_back(filter);
        for (size_t col = 0; col < _ihdr.width;) {
            uint8_t size = _ihdr.bit_depth;
            uint8_t cur_byte = data[offset];
            int cur_pos = 0;
            int cnt_bit_per_byte = 8;
            while (cur_pos < cnt_bit_per_byte && col < _ihdr.width) {
                new_data.push_back(read_ith_pixel_from_byte(cur_byte, cur_pos));
                col++;
                cur_pos += size;
            }
            offset++;
        }
    }
    data = new_data;
}

RGB PngDecoder::get_pixel_with_offset(const std::vector<uint8_t> &data, int offset) const {
    if (_ihdr.color_type == 3) {
        return _palette[data[offset]];
    } else if (_ihdr.color_type == 0) {
        return {(uint8_t) data[offset], (uint8_t) data[offset], (uint8_t) data[offset]};
    } else if (_ihdr.color_type == 4) {
        return {(uint8_t) data[offset], (uint8_t) data[offset], (uint8_t) data[offset], (uint8_t) data[offset + 1]};
    }
    RGB pixel = {(uint8_t) data[offset], (uint8_t) data[offset + 1], (uint8_t) data[offset + 2]};
    if (_ihdr.color_type == 6) {
        pixel.a = data[offset + 3];
    }
    return pixel;
}

Image PngDecoder::get_image_by_offset(std::vector<uint8_t> data, int &offset, size_t height, size_t width) const {
    Image image = Image((int) height, (int) width);
    for (int row = 0; row < image.Height(); row++) {
        int filter = data[offset];
        offset++;
        int bytes_per_pixel = (int) _ihdr.bytes_per_pixel();
        for (int col = 0; col < image.Width(); col++) {
            RGB &pixel = image(row, col);
            image(row, col) = get_pixel_with_offset(data, offset);

            PngFilter::filter(col, row, filter, image);
            if (_ihdr.color_type != 6 && _ihdr.color_type != 4) {
                pixel.a = 255;
            }
            pixel.r = (pixel.r + 256) % 256;
            pixel.g = (pixel.g + 256) % 256;
            pixel.b = (pixel.b + 256) % 256;
            pixel.a = (pixel.a + 256) % 256;
            offset += bytes_per_pixel;
        }
    }
    return image;
}