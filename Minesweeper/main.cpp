#include <cstdio>
#include <cstdlib>
#include <ctime>
#include "graphics.hpp"
#include "shared.hpp"
#include "solver.hpp"

// Counts number of mines. If 9, then it is a mine
int board[BOARDSIZE];

// cellState[loc] = 0 means unrevealed
// cellState[loc] = 1 means revealed
// cellState[loc] = 2 means flagged
unsigned int cellState[BOARDSIZE];

static int countAdjacentMines(int loc)
{
    int count = 0;
    for (const auto &rp : relPos)
    {
        int row = loc / NCOLS;
        int col = loc % NCOLS;
        
        row += rp.y;
        col += rp.x;
        
        if (col >= 0 && col < NCOLS && row >= 0 && row < NROWS)
            if (board[(row * NCOLS) + col] == 9)
                count++;
    }
    
    return count;
}

static void placeMines(int firstTile)
{
    int mineLocations[BOARDSIZE];
    
    // Reset to empty board
    for (int &cell : board)
        cell = -1;
    
    for (int i = 0; i < firstTile; i++)
        mineLocations[i] = i;
    
    for (int i = firstTile + 1; i < BOARDSIZE; i++)
        mineLocations[i - 1] = i;
    
    for (int i = 0; i < NMINES; i++)
    {
        int n = rand() % ((BOARDSIZE - 1) - i);
        
        board[mineLocations[n]] = 9;
        
        mineLocations[n] = mineLocations[(BOARDSIZE - 2) - i];
    }
    
    // Fill in the rest of the board accordingly
    for (int i = 0; i < BOARDSIZE; i++)
        if (board[i] != 9)
            board[i] = countAdjacentMines(i);
}

static int getTileNum(int x, int y)
{
    if (y <= 50)
        return -1;
    
    int rowNum = (y - 50) / TILE_HEIGHT;
    int colNum = x / TILE_WIDTH;
    return rowNum * NCOLS + colNum;
    
}

static int floodFill(int loc, int *queue)
{
    bool visited[BOARDSIZE] = {};
    int left = 0;
    int right = 0;
    
    queue[right++] = loc;
    visited[loc] = true;
    
    while (left < right)
    {
        int curLoc = queue[left++];
        if (board[curLoc] == 0)
        {
            for (const auto &rp : relPos)
            {
                int row = curLoc / NCOLS;
                int col = curLoc % NCOLS;
                
                row += rp.y;
                col += rp.x;
                
                if (col >= 0 && col < NCOLS && row >= 0 && row < NROWS)
                {
                    int newLoc = (row * NCOLS) + col;
                    
                    if (!visited[newLoc] && cellState[newLoc] != REVEALED)
                    {
                        queue[right++] = newLoc;
                        visited[newLoc] = true;
                    }
                }
            }
        }
    }
    
    return right;
}

int main(int argc, char* args[])
{
    srand((unsigned int)time(NULL));
    bool firstClick = true;
    int firstClickTicks = 0;
    bool doingReveal = false;
    bool gameOver = false;
    int revealIndex = 0;
    int revealCount = 0;
    int lastRevealTicks = 0;
    int floodFillQueue[BOARDSIZE];
    int nFlags = 0;
    int secs = 0;
    int unrevealedCount = BOARDSIZE;
    TextureStruct textures;
    Solver solver;
    Texture *currentFace = &(textures.happy);
    
    if(!init())
    {
        printf("Failed to initialize!\n");
    }
    else
    {
        if(!loadMedia(textures))
        {
            printf("Failed to load media!\n");
        }
        else
        {
            bool quit = false;
            
            SDL_Event e;
            
            while(!quit)
            {
                while(SDL_PollEvent(&e) != 0)
                {
                    if (e.type == SDL_QUIT)
                    {
                        quit = true;
                    }
                    else if (e.type == SDL_MOUSEBUTTONDOWN && !doingReveal)
                    {
                        // tileNum returns -1 if clicking on top bar.
                        // solver will change tileNum if it moves.
                        int tileNum = getTileNum(e.button.x, e.button.y);
                        
                        if (tileNum == -1)
                        {
                            // Click on face
                            if (e.button.x > SCREEN_WIDTH / 2 - 15
                                && e.button.x < SCREEN_WIDTH / 2 + 15)
                            {
                                printf("\nNew game\n");
                                secs = 0;
                                currentFace = &(textures.happy);
                                gameOver = false;
                                firstClick = true;
                                firstClickTicks = 0;
                                nFlags = 0;
                                solver.clearQueue();
                                
                                // Set all tiles to unrevealed/unflagged.
                                for (unsigned int &cell : cellState)
                                    cell = 0;
                                
                                unrevealedCount = BOARDSIZE;
                                continue;
                            }
                            // Click on light bulb
                            else if (!gameOver
                                     && e.button.x > SCREEN_WIDTH / 2 + 15
                                     && e.button.x < SCREEN_WIDTH / 2 + 45)
                            {
                                // solver.singleSquare() returns an idx to move, or if it's a flag
                                // it returns idx + BOARDSIZE
                                tileNum = solver.singleSquare();
                                if (tileNum == -1)
                                {
                                    printf("Multiple square analysis\n");
                                    tileNum = solver.multiSquare();
                                }
                            }
                        }
                        
                        if (tileNum != -1
                            && tileNum < BOARDSIZE
                            && e.button.button == SDL_BUTTON_LEFT
                            && cellState[tileNum] == 0
                            && !gameOver)
                        {
                            if (firstClick)
                            {
                                placeMines(tileNum);
                                firstClick = false;
                                firstClickTicks = SDL_GetTicks();
                            }
                            
                            // Click on a mine
                            if (board[tileNum] == 9)
                            {
                                currentFace = &(textures.dead);
                                for (int i = 0; i < BOARDSIZE; i++)
                                    if (!(cellState[i] & FLAGGED))
                                        cellState[i] = REVEALED;
                                gameOver = true;
                            }
                            
                            revealCount = floodFill(tileNum, floodFillQueue);
                            unrevealedCount -= revealCount;
                            if (unrevealedCount == NMINES)
                            {
                                currentFace = &(textures.glasses);
                                gameOver = true;
                            }
                            revealIndex = 0;
                            lastRevealTicks = 0;
                            doingReveal = true;
                        }
                        else if (tileNum >= BOARDSIZE ||
                                 (e.button.button == SDL_BUTTON_RIGHT
                                  && !(cellState[tileNum] & REVEALED)
                                  && !gameOver))
                        {
                            if (tileNum >= BOARDSIZE)
                                tileNum -= BOARDSIZE;
                            
                            if (cellState[tileNum] & FLAGGED)
                                nFlags--;
                            else
                                nFlags++;
                            
                            cellState[tileNum] ^= FLAGGED;
                        }
                    }
                }
                
                if (doingReveal)
                {
                    int curTicks = SDL_GetTicks();
                    
                    if (curTicks - lastRevealTicks >= 50)
                    {
                        int idx = floodFillQueue[revealIndex];
                        lastRevealTicks = curTicks;
                        
                        if (cellState[idx] & FLAGGED)
                            nFlags--;
                        
                        cellState[idx] = REVEALED;
                        
                        if (++revealIndex == revealCount)
                            doingReveal = false;
                    }
                }
                
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(renderer);
                
                if (!firstClick && !gameOver)
                {
                    secs = (SDL_GetTicks() - firstClickTicks) / 1000;
                    
                    if (secs > 999)
                        secs = 999;
                }
                
                // Display timer
                textures.counterNumbers[secs / 100].render(0, 0);
                textures.counterNumbers[(secs / 10) % 10].render(30, 0);
                textures.counterNumbers[secs % 10].render(60, 0);
                
                currentFace->render(SCREEN_WIDTH / 2 - 15, 0);
                textures.lightBulb.render(SCREEN_WIDTH / 2 + 15, -3);
                
                // Display supposed number of remaining mines
                int a = nFlags > NMINES ? NMINES : nFlags;
                textures.counterNumbers[(NMINES - a) / 100].render(SCREEN_WIDTH - 90, 0);
                textures.counterNumbers[(NMINES - a) / 10].render(SCREEN_WIDTH - 60, 0);
                textures.counterNumbers[(NMINES - a) % 10].render(SCREEN_WIDTH - 30, 0);
                
                for (int i = 0; i < BOARDSIZE; i++)
                {
                    int x = TILE_WIDTH * (i % NCOLS);
                    int y = 50 + TILE_HEIGHT * (i / NCOLS);
                    
                    if (cellState[i] & FLAGGED)
                    {
                        textures.flag.render(x, y);
                    }
                    else if (cellState[i] & REVEALED)
                    {
                        int n = board[i];
                        if (n == 9)
                            textures.mine.render(x, y);
                        else if (n > 0)
                            textures.numbers[n - 1].render(x, y);
                    }
                    else
                    {
                        textures.unrevealed.render(x, y);
                    }
                }
                
                for (int i = 0; i < NCOLS + 1; i++)
                {
                    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
                    SDL_RenderDrawLine(renderer, TILE_WIDTH * i, 50, TILE_WIDTH * i, SCREEN_HEIGHT);
                    SDL_RenderDrawLine(renderer, 0, 50 + TILE_HEIGHT * i, SCREEN_WIDTH, 50 + TILE_HEIGHT * i);
                }
                
                SDL_RenderPresent(renderer);
            }
        }
    }
    
    IMG_Quit();
    SDL_Quit();
    return 0;
}
