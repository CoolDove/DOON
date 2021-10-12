#pragma once
#include "stb_image/stb_image.h"

class Image {
public:
	Image(const char* _path, int _desired_channel);
	~Image();
	unsigned char* pixels_;
	struct Info {
		int width;
		int height;
		int channel;
	} info_;
};