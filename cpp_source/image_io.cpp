#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "image_io.h"

void write_png(const std::string& file, const int width, const int height, const int channels, const bool flip_image, const void* data)
{
	if(flip_image)
		stbi_flip_vertically_on_write(1);

	int ret = stbi_write_png(file.c_str(), width, height, channels, data, width * channels);
	if (!ret)
		fprintf(stderr, "Fail to write PNG!");

	stbi_flip_vertically_on_write(0);
}

void image_convert_f32_to_byte(const float* src_data, const int width, const int height, const int channels, uint8_t* dst_data)
{
	auto clamp_uint8 = [](float i){
		return i > 255.0f ? 255.0f : (i < 0.0f ? 0.0f : i);
	};

	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			const int index = y * width + x;
			const int channel_offset = index * channels;
			for (int channel_index = 0; channel_index < channels; ++channel_index) {
				int g_offset = channel_offset + channel_index;
				dst_data[g_offset] = (uint8_t)clamp_uint8(src_data[g_offset] * 255.0f);
			}
		}
	}

}