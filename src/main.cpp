#include <thread>

#include <SDL2/SDL.h>

#include "DoveLog.hpp"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <time.h>

// #include <SDL2/SDL_image.h>
#include <cstring>

#include "Application.h"

int main(int argc, char** args) {
	Application app;

	app.run();

	DLOG_TRACE("terminate");
	return 0;
}

static void draw_circle(SDL_Surface* _img, int _x, int _y, int _r, unsigned int _col) {
	auto px = [=](int _x, int _y){
		return _y * _img->w + _x;
	};

	int center = px(_x, _y);
 
	for (int i = 0; i < 2 * _r; i++)
	{
		int scan_length = 2 * glm::sqrt(_r * _r - (_r - i) * (_r - i));

		int start_x = _x - scan_length * 0.5f;
		int start_y = _y - _r + i;

		if (start_x > _img->w || start_x < 0 || start_y > _img->h || start_y < 0) {
			return;
		}
		scan_length = glm::min(scan_length, _img->w - start_x);
		int start = px(start_x, start_y);

		for (int j = 0; j < scan_length; j++)
		{
			int* pix = (int*)_img->pixels + start + j;
			*pix = _col;
		}
	}
}