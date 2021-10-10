#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <thread>

#include "DGLCore/DGLCore.h"

using namespace std;

class Application {
public:
	Application();
	~Application();
	void run();
	void terminate();
	SDL_Window* window;
	SDL_GLContext glcontext;

private:
	unordered_map<string, SDL_Surface*> images;
	unique_ptr<DGL::Shader> shader;
	unique_ptr<DGL::Camera> camera;
	DGL::GeoBatch* 	batch;
	GLuint img_id;
	glm::vec2 cam_pos;
	float cam_size;
private:
	bool terminated;

private:
	void init_dlog();
	void init_sdl();
	void init_opengl();
	void init_imgui();
private:
	void handle_event();
	void render();
	void draw_circle(SDL_Surface* _img, int _x, int _y, int _r, unsigned int _col);
};