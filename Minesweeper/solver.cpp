#include <cstdio>
#include <cstdlib>
#include <vector>
#include <utility>
#include "solver.hpp"
#include "shared.hpp"

// Counts number of adjacent unrevealed tiles, adjacent flags,
// and records which locations are unrevealed tiles
int Solver::countAdjacentUnrevealed(int loc, int &flagCount, bool adjacent[])
{
    int count = 0;
    for (int i = 0; i < 8; i++)
        adjacent[i] = false;
    
    for (int i = 0; i < 8; i++)
    {
        int row = loc / NCOLS;
        int col = loc % NCOLS;
        
        row += relPos[i].y;
        col += relPos[i].x;
        
        if (col >= 0 && col < NCOLS && row >= 0 && row < NROWS)
        {
            if (!cellState[row * NCOLS + col])
            {
                adjacent[i] = true;
                count++;
            }
            else if (cellState[row * NCOLS + col] & FLAGGED)
            {
                flagCount++;
            }
        }
    }
    return count;
}

int Solver::singleSquare()
{
    bool adjacent[8];
    for (int loc = 0; loc < BOARDSIZE; loc++)
    {
        // If the square is not revealed, skip to next
        if (!(cellState[loc] & REVEALED))
            continue;
        
        int flagCount = 0;
        int unrevealedCount = countAdjacentUnrevealed(loc, flagCount, adjacent);
        if (unrevealedCount)
        {
            // Then these all must be flags, so flag one of them
            if (board[loc] == unrevealedCount + flagCount)
            {
                // Find one that's unrevealed
                for (int i = 0; i < 8; i++)
                {
                    if (adjacent[i])
                    {
                        int col = (loc % NCOLS) + relPos[i].x;
                        int row = (loc / NCOLS) + relPos[i].y;
                        // For flagging, return loc + BOARDSIZE
                        return row * NCOLS + col + BOARDSIZE;
                    }
                }
            }
            // Then whatever is still unrevealed must be open
            else if (board[loc] == flagCount)
            {
                // Find one that's unrevealed
                for (int i = 0; i < 8; i++)
                {
                    if (adjacent[i])
                    {
                        int col = (loc % NCOLS) + relPos[i].x;
                        int row = (loc / NCOLS) + relPos[i].y;
                        return row * NCOLS + col;
                    }
                }
            }
        }
    }
    
    // Return -1 if nothing is found
    return -1;
}

bool Solver::isUnrevealedEdge(int loc)
{
    for (const auto &rp : relPos)
    {
        int row = loc / NCOLS;
        int col = loc % NCOLS;
        
        row += rp.y;
        col += rp.x;
        
        if (col >= 0 && col < NCOLS && row >= 0 && row < NROWS)
            if (cellState[row * NCOLS + col] & REVEALED)
                return true;
    }
    return false;
}

bool Solver::isRevealedEdge(int loc)
{
    for (const auto &rp : relPos)
    {
        int row = loc / NCOLS;
        int col = loc % NCOLS;
        
        row += rp.y;
        col += rp.x;
        
        if (col >= 0 && col < NCOLS && row >= 0 && row < NROWS)
            if (!cellState[row * NCOLS + col])
                return true;
    }
    return false;
}

int Solver::countAdjacentUnrevealed2(int loc)
{
    int count = 0;
    for (const auto &rp : relPos)
    {
        int row = loc / NCOLS;
        int col = loc % NCOLS;
        
        row += rp.y;
        col += rp.x;
        
        if (col >= 0 && col < NCOLS && row >= 0 && row < NROWS && !cellState[row * NCOLS + col])
            count += 1;
    }
    return count;
}

bool Solver::correctMineCount(int loc)
{
    int count = 0;
    int contributingConfigCount = 0;
    for (int i = 0; i < 8; i++)
    {
        int row = loc / NCOLS;
        int col = loc % NCOLS;
        
        row += relPos[i].y;
        col += relPos[i].x;
        
        if (col >= 0 && col < NCOLS && row >= 0 && row < NROWS)
        {
            int newLoc = row * NCOLS + col;
            if (cellState[newLoc] & FLAGGED)
            {
                count += 1;
            }
            else if (config.find(newLoc) != config.end())
            {
                if (config[newLoc])
                    count += 1;
                
                contributingConfigCount += 1;
            }
        }
    }
    
    // The second predicate  of the else if relies on !(count > board[loc]), so the above if needs to be there.
    if (count > board[loc])
        return false;
    // The second predicate is asking if there are enough adjacent tiles not currently in the configuration
    // for there to be mines in to make up for the difference between the current count and board[loc].
    else if (count == board[loc] || countAdjacentUnrevealed2(loc) - contributingConfigCount >= board[loc] - count)
        return true;
    
    return false;
}

bool Solver::isCompatibleConfig(std::vector<int> edgeRevealed)
{
    for (const int &loc : edgeRevealed)
        if (!correctMineCount(loc))
            return false;
    return true;
}

void Solver::findConfigs(std::vector<int> edgeUnrevealed, std::vector<int> edgeRevealed)
{
    if (edgeUnrevealed.size() == 0)
    {
        listOfConfigs.push_back(config);
        return;
    }
    
    for (int i = 0; i < 2; i++)
    {
        // edgeUnrevealed is cut at the beginning each recursive call so
        // that its 0th element is always the next spot to add to config.
        config[edgeUnrevealed[0]] = i;
        if (isCompatibleConfig(edgeRevealed))
        {
            // The size-1 vector case must be handled separately like this
            // because the edgeUnrevealed.begin() + 1 wouldn't make sense.
            std::vector<int> newUnrevealed;
            if (edgeUnrevealed.size() > 1)
                newUnrevealed = std::vector<int> (edgeUnrevealed.begin() + 1, edgeUnrevealed.end());
            
            findConfigs(newUnrevealed, edgeRevealed);
        }
        config.erase(edgeUnrevealed[0]);
    }
}

void Solver::clearQueue()
{
    while (!moves.empty())
        moves.pop();
}

int Solver::multiSquare()
{
    // See if there are any yet-to-be-made moves in queue from the last analysis
    while (!moves.empty())
    {
        int loc = moves.front();
        moves.pop();
        
        printf("Move from previous analysis\n");
        if (!cellState[loc > BOARDSIZE ? loc - BOARDSIZE : loc])
            return loc;
    }
    
    std::vector<int> edgeUnrevealed;
    std::vector<int> edgeRevealed;
    
    // Find all edge squares
    for (int loc = 0; loc < BOARDSIZE; loc++)
    {
        if (!cellState[loc])
        {
            if (isUnrevealedEdge(loc))
                edgeUnrevealed.push_back(loc);
        }
        else if (cellState[loc] & REVEALED)
        {
            if (isRevealedEdge(loc))
                edgeRevealed.push_back(loc);
        }
    }
    
    // Clear configs from previous analysis
    config.clear();
    listOfConfigs.clear();
    
    // Find all valid configurations
    findConfigs(edgeUnrevealed, edgeRevealed);
    
    // Used to count how many configurations have mines/open spaces
    // at each location.
    std::vector<int> commonElements(BOARDSIZE, 0);
    
    // Add 1 if open, add 2 if mine
    for (std::unordered_map<int, int> conf : listOfConfigs)
        for (std::pair<int,int> p : conf)
            commonElements[p.first] += p.second + 1;
    
    // Find locations that are open in each config or mine in each config
    for (int i = 0; i < BOARDSIZE; i++)
    {
        // Flag
        if (commonElements[i] == 2 * listOfConfigs.size())
            moves.push(i + BOARDSIZE);
        // Open space
        else if (commonElements[i] == listOfConfigs.size())
            moves.push(i);
    }
    
    // If any such locations were found
    if (!moves.empty())
    {
        int loc = moves.front();
        moves.pop();
        return loc;
    }
    
    // Move randomly if no move can be found
    printf("Moving randomly.\n");
    int idx;
    do
    {
        idx = rand() % BOARDSIZE;
    } while (cellState[idx]);
    
    return idx;
}
