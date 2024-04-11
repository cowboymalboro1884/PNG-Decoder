#include "filter.h"

/*
 * see http://www.libpng.org/pub/png/spec/1.2/PNG-Filters.html
*/

uint8_t PngFilter::paeth_predictor(uint8_t a, uint8_t b, uint8_t c) {
    int p = a + b - c;
    size_t pa = abs(p - a);
    size_t pb = abs(p - b);
    size_t pc = abs(p - c);
    if (pa <= pb && pa <= pc) return a;
    else if (pb <= pc) return b;
    return c;
}

void PngFilter::paeth_filter(Image &image, int x, int y) {
    RGB a_pixel = (x == 0) ? RGB{} : image(y, x - 1);
    RGB b_pixel = (y == 0) ? RGB{} : image(y - 1, x);
    RGB c_pixel = (x == 0 || y == 0) ? RGB{} : image(y - 1, x - 1);
    image(y, x) += {paeth_predictor(a_pixel.r, b_pixel.r, c_pixel.r),
                    paeth_predictor(a_pixel.g, b_pixel.g, c_pixel.g),
                    paeth_predictor(a_pixel.b, b_pixel.b, c_pixel.b),
                    paeth_predictor(a_pixel.a, b_pixel.a, c_pixel.a)};
}

void PngFilter::sub_filter(Image &image, int x, int y) {
    RGB a_pixel = (x == 0) ? RGB{} : image(y, x - 1);
    image(y, x) += a_pixel;
}

void PngFilter::up_filter(Image &image, int x, int y) {
    RGB b_pixel = (y == 0) ? RGB{} : image(y - 1, x);
    image(y, x) += b_pixel;
}

void PngFilter::average_filter(Image &image, int x, int y) {
    RGB a_pixel = (x == 0) ? RGB{} : image(y, x - 1);
    RGB b_pixel = (y == 0) ? RGB{} : image(y - 1, x);
    auto average = [](uint8_t a, uint8_t b) -> uint8_t {
        return std::floor((a + b) / 2);
    };

    image(y, x) += {average(a_pixel.r, b_pixel.r),
                    average(a_pixel.g, b_pixel.g),
                    average(a_pixel.b, b_pixel.b),
                    average(a_pixel.a, b_pixel.a),};
}

void PngFilter::filter(int x, int y, int filter, Image &image) {
    if (filter == 1) {
        sub_filter(image, x, y);
    } else if (filter == 2) {
        up_filter(image, x, y);
    } else if (filter == 3) {
        average_filter(image, x, y);
    } else if (filter == 4) {
        paeth_filter(image, x, y);
    }
}