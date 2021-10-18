#include "Core/Application.h"
#include "DoveLog.hpp"
#include <Core/Color.h>

int WinMain(HINSTANCE _instance, HINSTANCE _prev_instance, char* _cmd_line, int _show_code) {
    Application app(_instance, _prev_instance, _cmd_line, _show_code);
    app.run();
}