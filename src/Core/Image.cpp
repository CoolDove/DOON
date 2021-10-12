#include "Image.h"

Image::Image(const char* _path, int _desired_channel) {
	pixels_ = stbi_load(_path, &info.width, &info.height, &info.channel, _desired_channel);
}
Image::~Image() {
	stbi_image_free(pixels_);
}
