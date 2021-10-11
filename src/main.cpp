#include <thread>
// #include <SDL2/SDL.h>
#include <Windows.h>
#include "DoveLog.hpp"
// #include "Core/Application.h"
#include "string"
#include <glad/glad.h>
#include "Core/Application.h"

bool terminated = false;

int WinMain(HINSTANCE _instance, HINSTANCE _prev_instance, char* _cmd_line, int _show_code) {

    Application app(_instance, _prev_instance, _cmd_line, _show_code);
    app.run();

    return 0;
}