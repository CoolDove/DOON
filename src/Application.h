#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unordered_map>
#include <string>
#include <memory>

#include "DGLCore/DGLCore.h"

class Application {
public:
	Application();
	~Application();
	void run();
	void terminate();
	SDL_Window* window;
	SDL_GLContext glcontext;

private:
	std::unordered_map<std::string, SDL_Surface*> images;

	std::unique_ptr<DGL::Shader> shader;
	std::unique_ptr<DGL::Camera> camera;
	// std::unique_ptr<DGL::GeoBatch> batch;
	DGL::GeoBatch* batch;

	GLuint img_id;

	glm::vec2 cam_pos;
	float cam_size;

	// renderer
	// sessions
	// tool
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
};