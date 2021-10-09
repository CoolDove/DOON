#include "Application.h"
#include "DoveLog.hpp"
#include "Core/Input.h"

#include <assert.h>

#include <glad/glad.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <thread>


Application::Application() 
:	cam_pos(0),
	cam_size(5.0f)
{
	init_dlog();
	init_sdl();
	init_opengl();
	// init_imgui();

	camera = std::make_unique<DGL::Camera>();
	shader = std::make_unique<DGL::Shader>();
	batch = new DGL::GeoBatch({{DGL::Attribute::POSITION, 3}, { DGL::Attribute::UV, 2 }});

    /*──────────┐
    │ load data │
    └──────────*/
	shader->load("./res/shaders/base.vert", "./res/shaders/base.frag");
	shader->bind();

	images["jko"] = IMG_Load("res/textures/jko.png");
	images["test"] = IMG_Load("res/textures/test.png");
}

Application::~Application() {
}

void Application::run() {
	terminated = false;
	// do something
	static float col[4] = {0.2f,0.2f,0.2f,0};
	/*──────────────────────┐
	│ OpenGL Data preparing │
	└──────────────────────*/

	glCreateTextures(GL_TEXTURE_2D, 1, &img_id);
	glTextureStorage2D(img_id, 1, GL_RGBA12, images["jko"]->w, images["jko"]->h);

	glBindTextureUnit(0, img_id);
	glUniform1i(glGetUniformLocation(shader->get_id(), "_tex"), 0);

	batch->add_quad(0.5f * images["jko"]->w, 0.5f * images["jko"]->h, "quad");
	// canvas_size should be replaced by image size
	batch->upload();

	std::thread render_thd([this]{
		SDL_GL_MakeCurrent(window, glcontext);
		while(!terminated) {
			render();
		}
	});

	while (!terminated)
	{
		handle_event();
	}

	render_thd.join();
	// ImGui_ImplOpenGL3_Shutdown();
	// ImGui_ImplSDL2_Shutdown();
	// ImGui::DestroyContext();
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);

	SDL_Quit();
}
void Application::render() {
	// render here
	// glClearColor(col[0], col[1], col[2], col[3]);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/*─────────────┐
	│ OpenGL Layer │
	└─────────────*/
	glTextureSubImage2D(img_id, 0, 0, 0, images["jko"]->w, images["jko"]->h, GL_RGBA, GL_UNSIGNED_BYTE, images["jko"]->pixels);	

	glBindTexture(GL_TEXTURE_2D, img_id);

	batch->draw_batch();
	
	int uid_view_matrix = glGetUniformLocation(shader->get_id(), "_view");
	int uid_proj_matrix = glGetUniformLocation(shader->get_id(), "_proj");

	glm::mat4 view = camera->calc_view();

	int width, height;
	SDL_GetWindowSize(window, &width, &height);
	glm::mat4 proj = camera->calc_proj(width, height);

	glUniformMatrix4fv(uid_view_matrix, 1, false, &view[0][0]);
	glUniformMatrix4fv(uid_proj_matrix, 1, false, &proj[0][0]);

	// imgui layer
	// ImGui_ImplOpenGL3_NewFrame();
	// ImGui_ImplSDL2_NewFrame(window);
	// ImGui::NewFrame();

	// // ImGui::ShowDemoWindow();
	// {
	// 	ImGui::Begin("Camera");
	// 	ImGui::DragFloat2("position", &cam_pos[0], 0.01f, -1, 1);
	// 	ImGui::DragFloat("size", &cam_size, 0.01f, 0.1f, 10.0f);
	// 	ImGui::End();
	// }
	// camera->set_pos(cam_pos);
	// camera->set_size(cam_size);

	// ImGui::EndFrame();

	// ImGui::Render();
	// ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	SDL_GL_SwapWindow(window);
	SDL_Delay(16);
}

void Application::handle_event() {
	SDL_Event target_event;

	static bool mouse_holding = false;
	while (SDL_PollEvent(&target_event))
	{
		Core::Input::event_handle(&target_event);
		switch (target_event.type)
		{
			case SDL_QUIT:
				terminate();
				break;
			case SDL_WINDOWEVENT:
				switch (target_event.window.event)
				{
				case SDL_WINDOWEVENT_RESIZED:
					DLOG_INFO("resized");
					glViewport(0, 0, target_event.window.data1, target_event.window.data2);
					break;
				}
			case SDL_MOUSEMOTION:
				// calc the position in canvas space:
				if (mouse_holding) {
					int width, height;
					SDL_GetWindowSize(window, &width, &height);

					glm::mat4 matrix = camera->calc_view() * camera->calc_proj(width, height);
					matrix = glm::inverse(matrix);

					glm::vec2* mouse_pos = &Core::Input::Mouse::position;
					glm::vec4 ws_pos = glm::vec4(mouse_pos->x, mouse_pos->y, 1, 1);
					ws_pos.x = (ws_pos.x / width) * 2.0f - 1.0f;
					ws_pos.y = (ws_pos.y / height) * 2.0f - 1.0f;
					ws_pos.y *= -1;

					glm::vec4 cs_pos = matrix * ws_pos;

					int half_width = 0.5f * images["jko"]->w;
					int half_height = 0.5f * images["jko"]->h;

					// draw_circle(global_surface, cs_pos.x + half_width, -cs_pos.y + half_height, 30, 0xffffff00);
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

void Application::terminate() {
	terminated = true;
}

void Application::init_dlog() {
	DLOG_ON_PUSH = [](const Dove::LogMsg& _msg){
		printf("%s\n", _msg.to_string(Dove::DMSG_FLAG_SIMPLE | Dove::DMSG_FLAG_FILE | Dove::DMSG_FLAG_LINE).c_str());
	};

	DLOG_INIT;
}

void Application::init_sdl() {
	SDL_Init(SDL_INIT_VIDEO);

	window = nullptr;
	window = SDL_CreateWindow(
		"DOON",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		800, 600,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	assert(window != nullptr);
}

void Application::init_opengl() {
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
 
	glcontext = SDL_GL_CreateContext(window);

	// SDL_GL_MakeCurrent(window, glcontext);

	SDL_GL_SetSwapInterval(1);

	gladLoadGL();

}
void Application::init_imgui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForOpenGL(window, &glcontext);
	ImGui_ImplOpenGL3_Init(nullptr);

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	// ImFont* font = io.Fonts->AddFontFromFileTTF("...", 14.0f);
	DLOG_INFO("ImGui has been initialized");
}