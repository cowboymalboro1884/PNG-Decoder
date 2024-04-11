#include "image.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include "crc.h"
#include "filter.h"
#include "ihdr.h"
#include "decompressor.h"
#include "decoder.h"


inline Image ReadPng(std::string_view filename) {
    std::ifstream input_file(filename.data(), std::ios::binary);
    if (!input_file) {
        throw CouldNotOpenFile(std::string(filename));
    }

    return PngDecoder(input_file).process();
}
