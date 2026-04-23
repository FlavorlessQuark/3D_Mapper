#include <SDL2/SDL.h>
#include "../incl/visualizer.h"


/* 
    | z 
    |  / y 
    | / 
    |/_ _ _ _ _
             x

*/


int main(void) {
    SDL_Window *window;
    SDLX_Display *display;

    Vect3_Polar *p3D;
    Vect3 *pc3D = new Vect3[8];
    Vect2 *p2D = new Vect2[8];

    pc3D[0] = (Vect3){.x= -10, .y = 20, .z = 20};
    pc3D[1] = (Vect3){.x= -10, .y = 20, .z = 0};
    pc3D[2] = (Vect3){.x=  10, .y = 10, .z = 20};
    pc3D[3] = (Vect3){.x=  10, .y = 10, .z = 0};
    pc3D[4] = (Vect3){.x= -10, .y = 50, .z = 20};
    pc3D[5] = (Vect3){.x= -10, .y = 50, .z = 0};
    pc3D[6] = (Vect3){.x=  10, .y = 50, .z = 20};
    pc3D[7] = (Vect3){.x=  10, .y = 50, .z = 0};

    SDLX_InitDefault();

    display = SDLX_DisplayGet();
    
    while (1) {
        SDL_RenderClear(display->renderer);
		InputLoop();
        project(pc3D, p2D, 8);
        SDL_RenderPresent(display->renderer);
    }
}
