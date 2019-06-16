

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
#include <time.h>

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500

enum { BOMB = -1, EMPTY };
enum { HIDDEN, SHOWN, REVEALED, MARKED };


int handle_events(struct nk_context* ctx);
void reveal(int* board, int* board2, int i);

int main()
{
	SDL_Window* win;
	SDL_Renderer* ren;
	struct nk_context* ctx;

	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		printf("Can't init SDL:  %s\n", SDL_GetError( ));
		return 1;
	}

	win = SDL_CreateWindow("minesweeper",
                           SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                           WINDOW_WIDTH, WINDOW_HEIGHT,
                           SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);

    int scr_w = WINDOW_WIDTH;
    int scr_h = WINDOW_HEIGHT;

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

	int gui_flags = NK_WINDOW_NO_SCROLLBAR;

	struct nk_color background = { 0, 0, 0, 255 };

	srand(time(NULL));


	int board[100] = { EMPTY };
	int board2[100] = { HIDDEN };
	int tmp;
	for (int i=0; i<10; ++i) {
		tmp = rand() % 100;
		while (board[tmp] == BOMB)
			tmp = rand() % 100;
		board[tmp] = BOMB;
	}

	for (int i=0; i<10; ++i) {
		for (int j=0; j<10; ++j) {
			if (board[i*10+j] == BOMB) {
				int k = (i-1 < 0) ? 0 : i-1;
				int g = (j-1 < 0) ? 0 : j-1;
				int ek = (i+2 > 10) ? 10 : i+2;
				int eg = (j+2 > 10) ? 10 : j+2;

				int sg = g;
				//printf("%d %d, going %d %d to %d %d\n", i, j, k, g, ek, eg);
				for (; k<ek; ++k) {
					for (g=sg; g<eg; ++g) {
						//printf("%d %d\n", k, g);
						if (board[k*10+g] != BOMB)
							board[k*10+g]++;
					}
				}
			}
		}
	}

	char* nums[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8" };
	struct nk_color hidden_color = {128,128,128, 255};
	struct nk_color red = {128,0,0, 255};

	struct nk_rect bounds;

	//ctx->style.window.padding.x = 0;
	//ctx->style.window.padding.y = 0;
	struct nk_vec2 padding;
	ctx->style.button.padding.x = 0;
	ctx->style.button.padding.y = 0;
	padding.x = ctx->style.button.border = 0;
	padding.y = ctx->style.button.rounding = 0; // no rounded corners

	padding.x = ctx->style.window.spacing.x;
	padding.y = ctx->style.window.spacing.y;
	padding.x = ctx->style.window.padding.x;
	padding.y = ctx->style.window.padding.y;
	//struct nk_vec2 padding = nk_panel_get_padding(&ctx->style, NK_PANEL_WINDOW);
	//
	float wp = ctx->style.window.padding.x;

	while (1) {
		if (handle_events(ctx)) {
			break;
		}

		if (nk_begin(ctx, "Minesweeper", nk_rect(0, 0, 2*wp+10*(40)+11*padding.x, WINDOW_HEIGHT),
			gui_flags))
		{
			int i;

			nk_layout_row_static(ctx, 40, 40, 10);
			printf("padding = %.1f %.1f\n", padding.x, padding.y);
			for (i = 0; i < 100; ++i) {

				bounds = nk_widget_bounds(ctx);
				if (board2[i] != SHOWN) {
    				if (nk_input_mouse_clicked(&ctx->input, NK_BUTTON_RIGHT, bounds)) {
    					board2[i] = (board2[i] == HIDDEN) ? MARKED : HIDDEN;
    				}

					if (board2[i] == HIDDEN) {
						if (nk_button_color(ctx, hidden_color)) {
							reveal(board, board2, i);
						}
					} else {
						nk_button_symbol(ctx, NK_SYMBOL_CIRCLE_SOLID);
					}
				} else {
					if (board[i] != BOMB)
						nk_button_label(ctx, nums[board[i]]);
					else
						nk_button_color(ctx, red);
				}
			}
		}
		nk_end(ctx);

		nk_sdl_render(&background, SDL_TRUE);

		SDL_Delay(50);  // save cpu/battery, this isn't a real time game
	}


	nk_sdl_shutdown();
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;


}


void reveal(int* board, int* board2, int i)
{
	board2[i] = REVEALED;

	if (board[i] != EMPTY) {
		board2[i] = SHOWN;
		return;
	}
	
	while (1) {
		int j = i % 10;
		i = i / 10;
		
		int k = (i-1 < 0) ? 0 : i-1;
		int g = (j-1 < 0) ? 0 : j-1;
		int ek = (i+2 > 10) ? 10 : i+2;
		int eg = (j+2 > 10) ? 10 : j+2;

		int sg = g;
		//printf("%d %d, going %d %d to %d %d\n", i, j, k, g, ek, eg);
		for (; k<ek; ++k) {
			for (g=sg; g<eg; ++g) {
				//printf("%d %d\n", k, g);
				if (board2[k*10+g] == HIDDEN) {
					if (board[k*10+g] == EMPTY) {
						board2[k*10+g] = REVEALED;
					} else if (board[k*10+g] != BOMB) {
						board2[k*10+g] = SHOWN;
					}
				}
			}
		}
		board2[i*10+j] = SHOWN;

		for (i = 0; i<100; ++i) {
			if (board2[i] == REVEALED)
				break;
		}
		if (i == 100)
			break;
	}

}

int handle_events(struct nk_context* ctx)
{
	SDL_Event e;
	nk_input_begin(ctx);
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			return 1;
		case SDL_KEYUP:
			if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				return 1;
		}

		nk_sdl_handle_event(&e);
	}
	nk_input_end(ctx);

	return 0;
}




