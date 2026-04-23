#include <SDL2/SDL.h>

void InputLoop(void)
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
			exit(0);
		if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
			exit(0);
	}
}
