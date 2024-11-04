#include "World/PathfindingEngine.hpp"

void PathfindingEngine::resize(int width, int height)
{
    obstacleGrid = std::vector<char>(width * height, false);
    this->width = width;
    this->height = height;
}

void PathfindingEngine::setObstacle(int x, int y, bool solid)
{
    int gridIndex = getGridIndex(x, y);
    
    if (gridIndex < 0 || gridIndex >= width * height)
    {
        return;
    }

    obstacleGrid[gridIndex] = solid;
}

bool PathfindingEngine::findPath(int startX, int startY, int endX, int endY, std::vector<PathfindGridCoordinate>& result, bool straightening) const
{
    std::unordered_map<int, PathNode> pathNodes;

    std::priority_queue<int, std::vector<int>, PathNodeComparator> idxQueue{PathNodeComparator(pathNodes)};

    int startIdx = getGridIndex(startX, startY);
    int endIdx = getGridIndex(endX, endY);

    idxQueue.push(startIdx);
    pathNodes[startIdx] = PathNode{0, calculateHeuristic(startX, startY, endX, endY), -1};

    while (!idxQueue.empty())
    {
        int idx = idxQueue.top();
        if (idx == endIdx)
        {
            // End search
            result = retracePath(endIdx, pathNodes);
            return true;
        }

        // Remove from queue
        idxQueue.pop();

        const PathNode& previousNode = pathNodes[idx];

        int xIndex = idx % width;

        // if (xIndex > 0)
        // {

        int nextIdx = idx - 1;
        if (std::floor(idx / width) > std::floor(nextIdx / width) || nextIdx < 0)
        {
            nextIdx += width;
        }
        advancePathNode(nextIdx, idx, previousNode.pathCost, endIdx, 3, previousNode.direction, pathNodes, idxQueue, straightening);

        // }
        // if (xIndex < width - 1)
        // {
        nextIdx = idx + 1;
        if (std::floor(idx / width) < std::floor(nextIdx / width) || nextIdx >= obstacleGrid.size())
        {
            nextIdx -= width;
        }
        advancePathNode(nextIdx, idx, previousNode.pathCost, endIdx, 1, previousNode.direction, pathNodes, idxQueue, straightening);
        // }
        // if (idx >= width)
        // {

        nextIdx = idx - width;
        if (nextIdx < 0)
        {
            nextIdx += obstacleGrid.size();
        }
        advancePathNode(nextIdx, idx, previousNode.pathCost, endIdx, 0, previousNode.direction, pathNodes, idxQueue, straightening);
        // }
        // if (idx < obstacleGrid.size() - width)
        // {

        nextIdx = idx + width;
        if (nextIdx >= obstacleGrid.size())
        {
            nextIdx -= obstacleGrid.size();
        }
        advancePathNode(nextIdx, idx, previousNode.pathCost, endIdx, 2, previousNode.direction, pathNodes, idxQueue, straightening);    
        // }
    }

    // No path found
    return false;
}

int PathfindingEngine::getGridIndex(int x, int y) const
{
    return y * width + x;
}

int PathfindingEngine::calculateHeuristic(int x, int y, int destX, int destY) const
{
    return std::abs(destX - x) + std::abs(destY - y);
}

int PathfindingEngine::calculateHeuristic(int idx, int destIdx) const
{
    // int diff = std::abs(destIdx - idx);
    // return diff % width + std::floor(diff / width);
    int x = idx % width;
    int y = idx / width;
    int destX = destIdx % width;
    int destY = destIdx / width;
    return std::abs(destX - x) + std::abs(destY - y);
}

void PathfindingEngine::advancePathNode(int idx, int previousIdx, int previousPathCost, int destIdx, int direction, int previousDirection,
    std::unordered_map<int, PathNode>& pathNodes, std::priority_queue<int, std::vector<int>, PathNodeComparator>& idxQueue, bool straightening) const
{
    if (obstacleGrid[idx])
    {
        return;
    }

    int newCost = previousPathCost + 1;
    if (straightening)
    {
        if (direction != previousDirection)
        {
            newCost++;
        }
    }

    if (pathNodes.contains(idx))
    {
        PathNode& pathNode = pathNodes[idx];
        if (newCost < pathNode.pathCost)
        {
            pathNode.pathCost = newCost;
            pathNode.totalCost = newCost + calculateHeuristic(idx, destIdx);
            pathNode.direction = direction;
            pathNode.previousIdx = previousIdx;
            idxQueue.push(idx);
        }
        return;
    }

    pathNodes[idx] = PathNode{newCost, newCost + calculateHeuristic(idx, destIdx), direction, previousIdx};
    idxQueue.push(idx);
}

std::vector<PathfindGridCoordinate> PathfindingEngine::retracePath(int endIdx, const std::unordered_map<int, PathNode>& pathNodes) const
{
    std::vector<PathfindGridCoordinate> path;
    int currentIdx = endIdx;

    while (currentIdx >= 0)
    {
        PathfindGridCoordinate coordinate;
        coordinate.x = currentIdx % width;
        coordinate.y = std::floor(currentIdx / width);
        path.push_back(coordinate);

        currentIdx = pathNodes.at(currentIdx).previousIdx;
    }

    return path;
}