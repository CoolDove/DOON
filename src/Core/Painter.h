#pragma once
#include <SDL2/SDL.h>


using PainterColor = unsigned int;

class Painter {
public:
	Painter(SDL_Surface* _img);
	void set_color(PainterColor _col);

	void draw_circle(int _x, int _y, int _r);

private:
	PainterColor color_;
	SDL_Surface* image_;
};