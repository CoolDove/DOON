#include <list>
#include <stdio.h>
#include <thread>

#include <SDL2/SDL.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <glad/glad.h> // <GL/gl> has been included here
#include "DoveLog.hpp"

#include "DGLCore/GLBuffer.h"
#include "DGLCore/GLShader.h"
#include "DGLCore/GLCamera.h"
#include "DGLCore/GLGeoBatch.h"

#include "DSKCore/Input.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <time.h>

#include <SDL2/SDL_image.h>
#include <cstring>

using namespace DGL;

static void on_dlog_push(const Dove::LogMsg& _msg) {
	printf("%s\n", _msg.to_string(Dove::DMSG_FLAG_SIMPLE | Dove::DMSG_FLAG_FILE | Dove::DMSG_FLAG_LINE).c_str());
}
static void on_dlog_init() {
	DLOG_ON_PUSH = on_dlog_push;
}
//func prefab
static void handle_event();

// globals
static bool gs_run = true;
static SDL_Window* g_window;
static glm::vec2 window_size;

static Camera* global_camera;
static SDL_Surface* global_surface;

int main(int argc, char** args) {
	//Initialize all the systems of SDL
	// init DoveLog
	DLOG_ON_INIT = on_dlog_init;
	DLOG_INIT;

	// init SDL
	SDL_Init(SDL_INIT_VIDEO);

	g_window = nullptr;
	g_window = SDL_CreateWindow(
		"DoveWnd",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		800, 600,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	window_size.x = 800;
	window_size.y = 600;
	
	float canvas_width = 400;
	float canvas_height = 300;

	if (g_window == nullptr) return -1;

	// init GL and glad
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
 
	SDL_GLContext context = SDL_GL_CreateContext(g_window);
	SDL_GL_MakeCurrent(g_window, context);
	SDL_GL_SetSwapInterval(1);

	// int status = gladLoadGLLoader(SDL_GL_GetProcAddress);
	gladLoadGL();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForOpenGL(g_window, context);
	ImGui_ImplOpenGL3_Init(nullptr);

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	// ImFont* font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/ubuntu/Ubuntu-M.ttf", 14.0f);
	DLOG_INFO("ImGui has been initialized");
	
	static float col[4] = {0.2f,0.2f,0.2f,0};
	/*──────────────────────┐
	│ OpenGL Data preparing │
	└──────────────────────*/

	Shader shader;
	shader.load("./res/shaders/base.vert", "./res/shaders/base.frag");
	shader.bind();

	SDL_Surface* img_jko = IMG_Load("res/textures/jko.png");
	SDL_Surface* img_test = IMG_Load("res/textures/test.png");

	global_surface = img_jko;

	GLuint gl_img;
	glCreateTextures(GL_TEXTURE_2D, 1, &gl_img);
	glTextureStorage2D(gl_img, 1, GL_RGBA12, img_jko->w, img_jko->h);

	glBindTextureUnit(0, gl_img);
	glUniform1i(glGetUniformLocation(shader.get_id(), "_tex"), 0);

	GeoBatch batch{ {Attribute::POSITION, 3}, { Attribute::UV, 2 } };
	batch.add_quad(0.5f * img_jko->w, 0.5f * img_jko->h, "quad");
	// canvas_size should be replaced by image size
	batch.upload();

	Camera camera;
	global_camera = &camera;

	glm::vec2 cam_pos(0);
	float cam_size = 5.0f;

	while (gs_run)
	{
		handle_event();
		
		glClearColor(col[0], col[1], col[2], col[3]);
		glClear(GL_COLOR_BUFFER_BIT);
		/*─────────────┐
		│ OpenGL Layer │
		└─────────────*/

		glTextureSubImage2D(gl_img, 0, 0, 0, img_jko->w, img_jko->h, GL_RGBA, GL_UNSIGNED_BYTE, img_jko->pixels);	
		glTextureSubImage2D(gl_img, 0, 1024, 1024, img_jko->w, img_jko->h, GL_RGBA, GL_UNSIGNED_BYTE, img_jko->pixels);	

		glBindTexture(GL_TEXTURE_2D, gl_img);
		batch.draw_batch();
		
		int uid_view_matrix = glGetUniformLocation(shader.get_id(), "_view");
		int uid_proj_matrix = glGetUniformLocation(shader.get_id(), "_proj");

		glm::mat4 view = camera.calc_view();
		glm::mat4 proj = camera.calc_proj(window_size.x, window_size.y);

		glUniformMatrix4fv(uid_view_matrix, 1, false, &view[0][0]);
		glUniformMatrix4fv(uid_proj_matrix, 1, false, &proj[0][0]);

		// imgui layer
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(g_window);
		ImGui::NewFrame();

		// ImGui::ShowDemoWindow();
		{
			ImGui::Begin("Camera");
			ImGui::DragFloat2("position", &cam_pos[0], 0.01f, -1, 1);
			ImGui::DragFloat("size", &cam_size, 0.01f, 0.1f, 10.0f);
			ImGui::End();
		}
		camera.set_pos(cam_pos);
		camera.set_size(cam_size);

		// {
		// 	if (ImGui::BeginMainMenuBar()) {
		// 		if (ImGui::BeginMenu("Sys"))
		// 		{
		// 			if (ImGui::MenuItem("Open", "CTRL+F")) {}
		// 			if (ImGui::MenuItem("Exit", "")) {
		// 				gs_run = false;
		// 			} 
		// 			ImGui::EndMenu();
		// 		}
		// 		ImGui::EndMainMenuBar();
		// 	}
		// }

		ImGui::EndFrame();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		SDL_GL_SwapWindow(g_window);
		SDL_Delay(16);
	}
	//Close
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(g_window);

	SDL_Quit();

	return 0;
}

static void draw_circle(SDL_Surface* _img, int _x, int _y, int _r, int _col) {
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
		// memset((int*)_img->pixels + start, _col, sizeof(int) * scan_length);
	}
}

static void handle_event() {
	SDL_Event target_event;
	static bool mouse_holding = false;
	while (SDL_PollEvent(&target_event))
	{

		Core::Input::event_handle(&target_event);
		switch (target_event.type)
		{
			case SDL_QUIT:
				gs_run = false;
				break;
			case SDL_WINDOWEVENT:
				switch (target_event.window.event)
				{
				case SDL_WINDOWEVENT_RESIZED:
					DLOG_INFO("resized");
					window_size.x = target_event.window.data1;
					window_size.y = target_event.window.data2;
					glViewport(0, 0, target_event.window.data1, target_event.window.data2);
					break;
				}
			case SDL_MOUSEMOTION:
				// calc the position in canvas space:
				if (mouse_holding) {
					glm::mat4 matrix = global_camera->calc_view() * global_camera->calc_proj(window_size.x, window_size.y);
					matrix = glm::inverse(matrix);

                    /*────────────┐
                    │ to be fixed │
                    └────────────*/
					glm::vec2* mouse_pos = &Core::Input::Mouse::position;
					glm::vec4 ws_pos = glm::vec4(mouse_pos->x, mouse_pos->y, 1, 1);
					ws_pos.x = (ws_pos.x / window_size.x) * 2.0f - 1.0f;
					ws_pos.y = (ws_pos.y / window_size.y) * 2.0f - 1.0f;
					ws_pos.y *= -1;

					glm::vec4 cs_pos = matrix * ws_pos;

					int half_width = 0.5f * global_surface->w;
					int half_height = 0.5f * global_surface->h;
					draw_circle(global_surface, cs_pos.x + half_width, -cs_pos.y + half_height, 30, 0xff0000ff);

					// printf("x:%fy:%fz:%fw:%f\n", cs_pos.x, cs_pos.y, cs_pos.z, cs_pos.w);
					// printf("x:%fy:%fz:%fw:%f\n", ws_pos.x, ws_pos.y, cs_pos.z, cs_pos.w);
				}
			case SDL_MOUSEBUTTONDOWN:
				if (target_event.button.button == SDL_BUTTON_LEFT) 
					mouse_holding = true;
				break;
			case SDL_MOUSEBUTTONUP:
				if (target_event.button.button == SDL_BUTTON_LEFT) 
					mouse_holding = false;
				break;
			default:
				// do nothing
				break;
			ImGui_ImplSDL2_ProcessEvent(&target_event);
		}
	}
}