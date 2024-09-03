#include <stdio.h>
#include <SDL2/SDL.h>

#include <err.h>
#include "constants.h"
#include "powder.h"

int game_running;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int last_frame_time;

int hue_value = 0xff2323;
int brush_width = 20;

struct game game_state;

int initialize_window(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error initialising SDL.\n");
		return 0;
	}

	window = SDL_CreateWindow(
		NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_BORDERLESS
	);
	if (!window) {
		fprintf(stderr, "Error creating SDL Window.\n");
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	return 1;
}

void new_particle(int x, int y, int width, int colour) {
	//TODO: add error checking
	for (int i = -width/2; i <= width/2; i++) {
		for (int j = -width/2; j <= width/2; j++) {
			int rnd = rand() % 4;
			if (rnd < 4 && y+j > 0 && x+i > 0 && y+j < game_state.width && x+i < game_state.height) {
				if (game_state.grid[(x+i)*game_state.width + (y+j)] == 0) {
					game_state.grid[(x+i)*game_state.width + (y+j)] = colour;
				}
			}
		}
	}
}

void setup() {
	game_state.height = floor((double)WINDOW_HEIGHT/CELL_HEIGHT);
	game_state.width = floor((double)WINDOW_WIDTH/CELL_WIDTH);

	printf("R %d G %d B %d\n", (hue_value & 0xFF0000) >> 16, (hue_value & 0x00FF00) >> 8, hue_value & 0x0000FF);
	printf("game height %d x %d\n", game_state.height, game_state.width);
	for (int i = 0; i < game_state.height; i++) {
		for (int j = 0; j < game_state.width; j++) {
			game_state.grid[i*game_state.width + j] = 0;
		}
	}
}

void process_input() {
	int x, y;
	SDL_Event event;

	// if mouse down, spit out particle. Happens outside of event poll, so dragging can happen
	if (SDL_GetMouseState(&x, &y) & SDL_BUTTON_LMASK) {
		int adj_x = x/CELL_HEIGHT;
		int adj_y = y/CELL_WIDTH;
		if (adj_x > 0 && adj_x < game_state.width && adj_y > 0 && adj_y < game_state.height)
		{
			new_particle(adj_y, adj_x, brush_width, hue_value);
		}
	}

	while (SDL_PollEvent(&event)) {

		switch (event.type) {
			case SDL_QUIT:
				game_running = 0;
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE)
					game_running = 0;
				break;
			case SDL_MOUSEWHEEL:
				hue_value += event.wheel.y*100;
		}
	}
}

void update() {
	int new_grid[WINDOW_HEIGHT/CELL_HEIGHT * WINDOW_WIDTH/CELL_WIDTH];

	/* initialise grid */
	for (int i = 0; i < game_state.height; i++) {
		for (int j = 0; j < game_state.width; j++) {
			new_grid[i*game_state.width + j] = 0;
		}
	}

	/* update grid step */
	for (int i = 0; i < game_state.height; i++) {
		for (int j = 0; j < game_state.width; j++) {
			int state = game_state.grid[i*game_state.width + j];
			if (state > 0) {
				int below = game_state.grid[(i+1)*game_state.width + j];
				int dir = rand() % 10;
				if (dir > 5)
					dir = 1;
				else
					dir = -1;
				int A = game_state.grid[(i+1)*game_state.width + j+dir];
				int B = game_state.grid[(i+1)*game_state.width + j-dir];
				if ((i+1) >= game_state.height) {
					new_grid[i*game_state.width + j] = state;
				} else if (below == 0) {
					new_grid[(i+1)*game_state.width + j] = state;
				} else if (A == 0 && j+dir > 0 && j+dir < game_state.width) {
					new_grid[(i+1)*game_state.width + j+dir] = state;
				} else if (B == 0 && j-dir > 0 && j-dir < game_state.width) {
					new_grid[(i+1)*game_state.width + j-dir] = state;
				} else {
					new_grid[i*game_state.width + j] = state;
				}
			}
		}
	}

	/* repopulate grid */
	for (int i = 0; i < game_state.height; i++) {
		for (int j = 0; j < game_state.width; j++) {
			game_state.grid[i*game_state.width + j] = new_grid[i*game_state.width + j];
		}
	}
}

void render() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	for (int i = 0; i < game_state.height; i++) {
		for (int j = 0; j < game_state.width; j++) {
			int state = game_state.grid[i*game_state.width + j];
			if (state > 0) {
				SDL_Rect particle = {
					j*CELL_HEIGHT,
					i*CELL_HEIGHT,
					CELL_WIDTH,
					CELL_HEIGHT
				};
				SDL_SetRenderDrawColor(renderer, (state & 0xFF0000) >> 16, (state & 0x00FF00) >> 8, (state & 0x0000FF), 255);
				SDL_RenderFillRect(renderer, &particle);
			}
		}
	}

	SDL_RenderPresent(renderer);
}

void destroy_window() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main() {
	game_running = initialize_window();

	setup();

	while (game_running) {
		if (SDL_TICKS_PASSED(SDL_GetTicks(), last_frame_time + FRAME_TARGET_TIME)) {
			process_input();
			last_frame_time = SDL_GetTicks();
			update();
			render();
		}
	}

	destroy_window();

	return 0;
}
