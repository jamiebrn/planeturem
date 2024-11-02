#pragma once

#include <vector>
#include <queue>
#include <functional>
#include <cmath>
#include <unordered_map>

#include <SFML/Graphics.hpp>

struct PathfindGridCoordinate
{
    int x;
    int y;
};

class PathfindingEngine
{
public:
    PathfindingEngine() = default;

    void resize(int width, int height);

    void setObstacle(int x, int y, bool solid);

    bool findPath(int startX, int startY, int endX, int endY, std::vector<PathfindGridCoordinate>& result);

    inline const std::vector<char>& getObstacles() {return obstacleGrid;}

private:
    int getGridIndex(int x, int y);

    int calculateHeuristic(int x, int y, int destX, int destY);
    int calculateHeuristic(int idx, int destIdx);

    std::vector<int> getNeighbours(int idx);

    std::vector<PathfindGridCoordinate> retracePath(int endIdx, const std::unordered_map<int, int> previousIndexes);

    class CostComparator
    {
    public:
        CostComparator(const std::unordered_map<int, int>& _costs) : costs(_costs) {}

        bool operator()(int i, int j) const
        {
            return costs.at(i) > costs.at(j);
        }

    private:
        const std::unordered_map<int, int>& costs;
    };

private:
    std::vector<char> obstacleGrid;

    int width = 0;
    int height = 0;

};