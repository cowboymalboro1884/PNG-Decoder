#pragma once

#include "image.h"
#include <cmath>

inline RGB &operator+=(RGB &a, RGB other) {
    a.r += other.r;
    a.g += other.g;
    a.b += other.b;
    a.a += other.a;
    return a;
}

class PngFilter {
    static uint8_t paeth_predictor(uint8_t a, uint8_t b, uint8_t c);

    static void paeth_filter(Image &image, int x, int y);

    static void sub_filter(Image &image, int x, int y);

    static void up_filter(Image &image, int x, int y);

    static void average_filter(Image &image, int x, int y);

public:
    static void filter(int x, int y, int filter, Image &image);
};