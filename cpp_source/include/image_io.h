#ifndef IMAGE_IO_H
#define IMAGE_IO_H

#include <string>

void write_png(const std::string& file, const int width, const int height, const int channels, const bool flip_image, const void* data);

void image_convert_f32_to_byte(const float *src_data, const int width, const int height, const int channels, uint8_t *dst_data);

#endif