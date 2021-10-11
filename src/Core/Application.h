#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <thread>
#include <Windows.h>

#include "DGLCore/DGLCore.h"

using namespace std;


class Application {
public:
	Application(HINSTANCE _instance, HINSTANCE _prev_instance, char* _cmd_line, int _show_code);
	~Application();
	void run();
	void terminate();

	static Application* get_instance() { return instance_; };
	static Application* instance_;

public:
	struct WindowInfo{
		int width;
		int height;
	} window_info_;

public:
	void handle_event();
	void render();
	// void draw_circle(SDL_Surface* _img, int _x, int _y, int _r, unsigned int _col);

private:
	// unordered_map<string, SDL_Surface*> images;
	unique_ptr<DGL::Shader> shader;
	unique_ptr<DGL::Camera> camera;
	DGL::GeoBatch* 	batch;
	GLuint img_id;
	glm::vec2 cam_pos;
	float cam_size;

	HWND window_;

	HDC device_context_;
	HGLRC gl_context_;
	
private:
	bool terminated;

private:
	void init_dlog();
	// void init_sdl();
	void init_window(HINSTANCE _instance, HINSTANCE _prev_instance, char* _cmd_line, int _show_code);
	void init_opengl();
	void init_imgui();

};
Application* get_app();