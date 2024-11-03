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

    bool findPath(int startX, int startY, int endX, int endY, std::vector<PathfindGridCoordinate>& result, bool straightening = false);

    inline const std::vector<char>& getObstacles() {return obstacleGrid;}

private:
    int getGridIndex(int x, int y);

    int calculateHeuristic(int x, int y, int destX, int destY);
    int calculateHeuristic(int idx, int destIdx);

    // std::vector<int> getNeighbours(int idx);

    struct PathNode
    {
        int pathCost;
        int totalCost;
        int direction;

        int previousIdx = -1;
    };

    class PathNodeComparator
    {
    public:
        PathNodeComparator(const std::unordered_map<int, PathNode>& _nodes) : nodes(_nodes) {}

        bool operator()(int i, int j) const
        {
            const PathNode& a = nodes.at(i);
            const PathNode& b = nodes.at(j);
            if (a.totalCost == b.totalCost)
            {
                return a.direction > b.direction;
            }
            return a.totalCost > b.totalCost;
        }

    private:
        const std::unordered_map<int, PathNode>& nodes;
    };

    void advancePathNode(int idx, int previousIdx, int previousPathCost, int direction, int previousDirection, int destIdx,
        std::unordered_map<int, PathNode>& pathNodes, std::priority_queue<int, std::vector<int>, PathNodeComparator>& idxQueue, bool straightening);

    std::vector<PathfindGridCoordinate> retracePath(int endIdx, const std::unordered_map<int, PathNode>& pathNodes);

private:
    std::vector<char> obstacleGrid;

    int width = 0;
    int height = 0;

};