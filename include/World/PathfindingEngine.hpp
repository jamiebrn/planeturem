#pragma once

#include <vector>
#include <queue>
#include <functional>
#include <cmath>
#include <unordered_map>
#include <optional>

#include "Core/Helper.hpp"
#include "Object/WorldObject.hpp"

// #include <SFML/Graphics.hpp>

#include "Vector.hpp"

struct PathfindGridCoordinate
{
    int x = 0;
    int y = 0;
};

class PathfindingEngine
{
public:
    PathfindingEngine() = default;

    void resize(int width, int height);

    void setObstacle(int x, int y, bool solid);

    bool findPath(int startX, int startY, int endX, int endY, std::vector<PathfindGridCoordinate>& result, bool straightening = false,
        std::optional<int> maxDistance = std::nullopt) const;

    std::vector<PathfindGridCoordinate> createStepSequenceFromPath(const std::vector<PathfindGridCoordinate>& path) const;

    PathfindGridCoordinate findFurthestOpenTile(int x, int y, int maxSearchRange, bool coordinateRelativeToStart = false) const;

    inline const std::vector<char>& getObstacles() const {return obstacleGrid;}
    inline int getWidth() const {return width;}
    inline int getHeight() const {return height;}

private:
    int getGridIndex(int x, int y) const;
    PathfindGridCoordinate getGridCoordinate(int gridIndex) const;

    int calculateHeuristic(int x, int y, int destX, int destY) const;
    int calculateHeuristic(int idx, int destIdx) const;

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
        std::unordered_map<int, PathNode>& pathNodes, std::priority_queue<int, std::vector<int>, PathNodeComparator>& idxQueue, bool straightening,
        std::optional<int> maxDistance) const;

    std::vector<PathfindGridCoordinate> retracePath(int endIdx, const std::unordered_map<int, PathNode>& pathNodes) const;

private:
    std::vector<char> obstacleGrid;

    int width = 0;
    int height = 0;

};

class PathFollower
{
public:
    PathFollower() = default;

    void beginPath(pl::Vector2f startPos, const std::vector<PathfindGridCoordinate>& pathfindStepSequence);

    pl::Vector2f updateFollower(float speed);

    bool isActive();

    void handleWorldWrap(pl::Vector2f positionDelta);

private:
    void setPathfindStepIndex(int index);

private:
    std::vector<PathfindGridCoordinate> stepSequence;
    pl::Vector2f position;
    pl::Vector2f lastStepPosition;
    pl::Vector2f stepTargetPosition;
    int stepIndex = 0;

    static constexpr float TARGET_REACH_THRESHOLD = 2.0f;

};