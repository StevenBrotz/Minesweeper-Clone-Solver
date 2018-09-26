#ifndef solver_hpp
#define solver_hpp

#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include "shared.hpp"

class Solver
{
public:
    int singleSquare();
    int multiSquare();
    void clearQueue();
private:
    std::queue<int> moves;
    std::unordered_map<int, int> config;
    std::vector<std::unordered_map<int, int>> listOfConfigs;
    int countAdjacentUnrevealed(int loc, int &flagCount, bool adjacent[]);
    int countAdjacentUnrevealed2(int loc);
    bool isUnrevealedEdge(int loc);
    bool isRevealedEdge(int loc);
    void findConfigs(std::vector<int> edgeUnrevealed, std::vector<int> edgeRevealed);
    bool correctMineCount(int loc);
    bool isCompatibleConfig(std::vector<int> edgeRevealed);
};

#endif /* solver_hpp */
