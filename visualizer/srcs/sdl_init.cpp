#include "../incl/structs.h"
#include "../incl/macros.h"

static	SDLX_Display display;

void	SDLX_Close(void)
{
	SDL_Log("Cleaning up...");
	SDL_DestroyWindow(display.window);

	SDL_Quit();
}

static void		SDLX_DisplaySet(char *name, int x, int y, int h, int w, int flags)
{
    SDL_Window *window;

	window = SDL_CreateWindow(
        name,
        x, y,
        w, h,
        flags
	);
    display.window = window;
    display.renderer = SDL_CreateRenderer(display.window, -1, 0);
    display.win_w = w;
    display.win_h = h;
	display.bgColor = (SDL_Color){0,0,0,255};
}

void SDLX_InitDefault()
{
	if (!SDL_WasInit(0))
		SDL_Init(SDL_INIT_EVERYTHING);

	SDLX_DisplaySet(
		DEFAULT_WIN_NAME,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        DEFAULT_WIN_H,
		DEFAULT_WIN_W,
        DEFAULT_SDL_FLAG
	);
	SDL_Log("Render queues initialized");
	atexit(SDLX_Close);
}

void	SDLX_Init(char *name, int x, int y, int h, int w, int flags)
{
	if (!SDL_WasInit(0))
		SDL_Init(SDL_INIT_EVERYTHING);

	SDLX_DisplaySet(name, x, y, h, w, flags);
	atexit(SDLX_Close);
}

SDLX_Display	*SDLX_DisplayGet(void)
{
    if (!display.renderer)
        return NULL;

    return &display;
}
