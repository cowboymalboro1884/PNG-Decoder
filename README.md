# PNG-decoder
## Overview
This project is a PNG image decoder that supports grayscale images, regular RGB, fixed-palette indexed images, and alpha transparency. It decodes PNG images using a pipeline of steps that includes reading signature bytes, parsing the fragments until an IEND fragment is found, checking the CRC of each fragment, concatenating the contents of all IDAT fragments into a single byte vector, and decoding the byte vector using deflation. The decoded data is processed by scanning line by line using specified filters.

## Usage
To use this decoder, include `png_decoder.h` in your project and call the ReadPng function with the name of the PNG image file as an argument. The function returns an Image object that contains the width, height, pixel format, and pixel data of the decoded image.

## Dependencies
- https://github.com/ebiggers/libdeflate (for deflating data)
- boost (for CRC)
