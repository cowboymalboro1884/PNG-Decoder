set(PNG_DECODER_SOURCES
        PngDecoder/src/decoder.cpp
        PngDecoder/src/filter.cpp
        PngDecoder/src/ihdr.cpp
        PngDecoder/src/decompressor.cpp
        PngDecoder/src/crc.cpp
)

include_directories(PngDecoder/include)
add_library(png_decoder_lib ${PNG_DECODER_SOURCES})

find_library(DEFLATE NAMES libdeflate.a REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)

target_link_libraries(png_decoder_lib Boost::system)
target_link_libraries(png_decoder_lib ${DEFLATE})

target_include_directories(png_decoder_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set(PNG_STATIC png_decoder_lib)
