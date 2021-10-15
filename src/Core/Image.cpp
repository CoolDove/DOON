#include "Image.h"

Image::Image(const char* _path, int _desired_channel) {
	pixels_ = stbi_load(_path, &info_.width, &info_.height, nullptr, _desired_channel);
}

Image::Image(unsigned int _width, unsigned int _height, unsigned int _base_color) {
	// pixels_ = stbi_load(_path, &info_.width, &info_.height, &info_.channel, _desired_channel);
	info_.width  = _width;
	info_.height = _height;

	pixels_ = (unsigned char*)malloc(_width * _height);

	// temp
	unsigned char col[4] = {
		(_base_color & 0xff000000)>>24,
		(_base_color & 0x00ff0000)>>16,
		(_base_color & 0x0000ff00)>> 8,
		(_base_color & 0x000000ff)>> 0,
	};

	int chan = 0;
	for (int i = 0; i < _width * _height; i++) {
		pixels_[i] = col[chan];
		chan = (chan+1)%4;
	}
}

Image::~Image() {
	free(pixels_);
	// stbi_image_free(pixels_);
}
