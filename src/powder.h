#include "constants.h"

#define CELL_WIDTH 3
#define CELL_HEIGHT 3

struct grid {
    int object_id;
};

struct game {
    int height;
    int width;
    int grid[WINDOW_HEIGHT/CELL_HEIGHT * WINDOW_WIDTH/CELL_WIDTH];
};
