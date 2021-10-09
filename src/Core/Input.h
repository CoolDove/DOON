#include <SDL2/SDL_main.h>
#include <glm/vec2.hpp>

namespace Core::Input 
{
namespace Mouse 
{
glm::vec2 position = {0, 0};
bool press_left = false;
bool press_right = false;
bool press_middle = false;
bool press_four = false;
bool press_five = false;
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