#include <thread>
// #include <SDL2/SDL.h>
#include <Windows.h>
#include "DoveLog.hpp"
#include "Core/Application.h"
#include "string"

LRESULT CALLBACK windows_proc(HWND _window, UINT _message, WPARAM _w_param, LPARAM _l_param);

bool terminated = false;

int WinMain(HINSTANCE _instance, HINSTANCE _prev_instance, char* _cmd_line, int _show_code) {
	std::string wnd_name = "DOON";

	WNDCLASS wnd_class = {};
	wnd_class.style = CS_HREDRAW | CS_VREDRAW;
	wnd_class.lpfnWndProc = windows_proc;
	wnd_class.hInstance = _instance;
	wnd_class.lpszClassName = "DOOMWindowClass";

	RegisterClass(&wnd_class);

	HWND window = CreateWindowEx(0,
								 wnd_class.lpszClassName, 
								 "DOOM", 
								 WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
								 CW_USEDEFAULT, 
								 CW_USEDEFAULT,
								 CW_USEDEFAULT, 
								 CW_USEDEFAULT,
								 nullptr, 
								 nullptr,
								 _instance,
								 nullptr);

	if (!window) {
		MessageBox(nullptr, "error occured while creating windows", "error", MB_OK);
		return -1;
	} else {
		// ShowWindow(window, _show_code);
		// UpdateWindow(window);
		while (!terminated)
		{
			MSG msg;
			while (BOOL result = GetMessage(&msg, nullptr, 0, 0)) {
				if (result > 0) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				} else {
					break;
				}
			}
		}
	}
	
	// old code
	// Application app;
	// app.run();
	return 0;
}

LRESULT CALLBACK windows_proc(HWND _window, UINT _message, WPARAM _w_param, LPARAM _l_param) {
	LRESULT result = 0;
	switch (_message)
	{
		case WM_SIZE:
		{
			OutputDebugString("resize\n");
		} break;
		case WM_DESTROY:
		{
			OutputDebugString("destroy\n");
			PostQuitMessage(0);
			terminated = true;
		} break;
		case WM_CLOSE:
		{
			OutputDebugString("close\n");
			PostQuitMessage(0);
			terminated = true;
		} break;
		case WM_ACTIVATEAPP:
		{
			OutputDebugString("activate\n");
		} break;
		default:
		{
			result = DefWindowProc(_window, _message, _w_param, _l_param);
			// need this to handle WM_CREATE and rerurn a non-zero value, 
			// otherwise we would failed to create a window
		} break;
	}
	return result;
}