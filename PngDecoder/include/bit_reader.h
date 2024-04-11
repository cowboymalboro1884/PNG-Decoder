#pragma once

#include <cstring>
#include <algorithm>

struct BitReader {
    template<class T>
    static T reverse_bytes(T value) {
        auto ptr = reinterpret_cast<char *>(&value);
        size_t l = 0;
        auto r = sizeof(T) - 1;
        while (l < r) {
            std::swap(ptr[l++], ptr[r--]);
        }
        value = *reinterpret_cast<T *>(ptr);
        return value;
    }

    template<class T>
    static T read_bytes_reversed(T *ptr) {
        T buf;
        std::memcpy(&buf, ptr, sizeof(T));
        return reverse_bytes(buf);
    }
};