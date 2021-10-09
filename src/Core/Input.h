#pragma once
#include <SDL2/SDL_main.h>
#include <glm/vec2.hpp>

namespace Core::Input 
{
namespace Mouse 
{
inline glm::vec2 position = {0, 0};
inline bool press_left = false;
inline bool press_right = false;
inline bool press_middle = false;
inline bool press_four = false;
inline bool press_five = false;
}


inline void event_handle(const SDL_Event* _event) {
	switch (_event->type)
	{
	case SDL_MOUSEMOTION:
		Mouse::position.x = _event->motion.x;
		Mouse::position.y = _event->motion.y;
		break;
	default:
		break;
	}
}
}