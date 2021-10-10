#include "Painter.h"
#include "glm/glm.hpp"

Painter::Painter(SDL_Surface* _img)
:	color_(0xffffffff),
	image_(_img)
{
}

void Painter::set_color(PainterColor _col) {
	color_ = _col;
}

void Painter::draw_circle(int _x, int _y, int _r) {
	auto px = [=](int _x, int _y){
		return _y * image_->w + _x;
	};

	int center = px(_x, _y);
 
	for (int i = 0; i < 2 * _r; i++)
	{
		int scan_length = 2 * glm::sqrt(_r * _r - (_r - i) * (_r - i));

		int start_x = _x - scan_length * 0.5f;
		int start_y = _y - _r + i;

		if (start_x > image_->w || start_x < 0 || start_y > image_->h || start_y < 0) {
			return;
		}
		scan_length = glm::min(scan_length, image_->w - start_x);
		int start = px(start_x, start_y);

		for (int j = 0; j < scan_length; j++)
		{
			int* pix = (int*)image_->pixels + start + j;
			*pix = color_;
		}
	}

}

