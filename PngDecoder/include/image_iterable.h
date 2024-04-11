#pragma once

#include "image.h"

struct ImageIterable {
private:
    Image current;
    int x = 0;
    int y = 0;
public:
    explicit ImageIterable(Image &im) : current(im) {}

    RGB next() {
        RGB tmp = current(y, x);
        x++;
        if (x == current.Width()) {
            y++;
            x = 0;
        }
        return tmp;
    }
};
