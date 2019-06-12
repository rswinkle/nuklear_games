

// need to have this before nuklear.h is a bug in nuklear
#include <stdarg.h>

// TODO sin, cos, sqrt etc.
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_STANDARD_IO
//#define NK_INCLUDE_FONT_BAKING
//#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl.h"



#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600


int handle_events(struct nk_context* ctx);

int main()
{
	SDL_Window* win;
	SDL_Renderer* ren;
	struct nk_context* ctx;

	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		printf( "Can't init SDL:  %s\n", SDL_GetError( ) );
		return 1;
	}

	win = SDL_CreateWindow("minesweeper",
                           SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                           WINDOW_WIDTH, WINDOW_HEIGHT,
                           SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);

	if (!win) {
		printf("Can't create window: %s\n", SDL_GetError());
		return 1;
	}
	/* try VSYNC and ACCELERATED */
	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
	if (!ren) {
		printf("Can't create renderer: %s\n", SDL_GetError());
		return 1;
	}

	if (!(ctx = nk_sdl_init(win, ren, 1, 1))) {
		printf("nk_sdl_init() failed!");
		return 1;
	}


	struct nk_color background = { 0, 0, 0, 255 };

	while (1) {
		if (handle_events(ctx)) {
			break;
		}

		if (nk_begin(ctx, "Minesweeper", nk_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT),
			NK_WINDOW_BORDER|NK_WINDOW_TITLE))
		{
			int i;
			static int selected[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

			nk_layout_row_static(ctx, 50, 50, 4);
			for (i = 0; i < 16; ++i) {
				if (nk_selectable_label(ctx, "Z", NK_TEXT_CENTERED, &selected[i])) {
					int x = (i % 4), y = i / 4;
					if (x > 0) selected[i - 1] ^= 1;
					if (x < 3) selected[i + 1] ^= 1;
					if (y > 0) selected[i - 4] ^= 1;
					if (y < 3) selected[i + 4] ^= 1;
				}
			}
		}
		nk_end(ctx);

		nk_sdl_render(&background, SDL_TRUE);
	}


	nk_sdl_shutdown();
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;


}

int handle_events(struct nk_context* ctx)
{
	SDL_Event e;
	nk_input_begin(ctx);
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT)
			return 1;
		nk_sdl_handle_event(&e);
	}
	nk_input_end(ctx);

	return 0;
}




