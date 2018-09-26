#ifndef shared_hpp
#define shared_hpp

#include "graphics.hpp"

const int TILE_WIDTH = 30;
const int TILE_HEIGHT = 30;
const int NMINES = 40;
const int NCOLS = 16;
const int NROWS = 16;
const int SCREEN_WIDTH = TILE_WIDTH * NCOLS;
const int SCREEN_HEIGHT = TILE_HEIGHT * NROWS + 50;
const int BOARDSIZE = NCOLS * NROWS;

const int REVEALED = 1;
const int FLAGGED = 2;

// Counts number of mines. If 9, then it is a mine
extern int board[BOARDSIZE];

// cellState[loc] = 0 means unrevealed
// cellState[loc] = 1 means revealed
// cellState[loc] = 2 means flagged
extern unsigned int cellState[BOARDSIZE];

// The window renderer
extern SDL_Renderer *renderer;

// Used to loop over adjacent cells
struct Offset {
    int x, y;
};
const Offset relPos[8] = {{-1, -1}, {0, -1}, {1, -1},
                          {-1,  0},          {1,  0},
                          {-1,  1}, {0,  1}, {1,  1}};


#endif /* shared_hpp */
