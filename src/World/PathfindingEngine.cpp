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

bool PathfindingEngine::findPath(int startX, int startY, int endX, int endY, std::vector<PathfindGridCoordinate>& result)
{
    std::unordered_map<int, int> pathCosts;
    std::unordered_map<int, int> totalCosts;
    std::unordered_map<int, int> previousIndexes;

    std::priority_queue<int, std::vector<int>, CostComparator> idxQueue{CostComparator(totalCosts)};
    std::unordered_map<int, bool> idxesInQueue;

    int startIdx = getGridIndex(startX, startY);
    int endIdx = getGridIndex(endX, endY);

    idxQueue.push(startIdx);
    idxesInQueue[startIdx] = true;
    pathCosts[startIdx] = 0;
    totalCosts[startIdx] = calculateHeuristic(startX, startY, endX, endY);

    while (!idxQueue.empty())
    {
        int lowestCostIdx = idxQueue.top();
        if (lowestCostIdx == endIdx)
        {
            // End search
            result = retracePath(endIdx, previousIndexes);
            return true;
        }

        // Remove from queue
        idxQueue.pop();
        idxesInQueue[lowestCostIdx] = false;

        for (int neighbour : getNeighbours(lowestCostIdx))
        {
            // If neighbour is obstacle, stop
            if (obstacleGrid[neighbour])
            {
                continue;
            }

            int nextPathCost = pathCosts[lowestCostIdx] + 1;

            bool canAdvance = true;
            if (pathCosts.contains(neighbour))
            {
                if (nextPathCost >= pathCosts[neighbour])
                {
                    canAdvance = false;
                }
            }

            if (canAdvance)
            {
                // More optimal path
                previousIndexes[neighbour] = lowestCostIdx;
                pathCosts[neighbour] = nextPathCost;
                totalCosts[neighbour] = nextPathCost + calculateHeuristic(neighbour, endIdx);

                // If neighbour is not in queue, add
                bool addToQueue = false;

                if (idxesInQueue.contains(neighbour))
                {
                    addToQueue = !idxesInQueue.at(neighbour);
                }
                else
                {
                    addToQueue = true;
                }

                if (addToQueue)
                {
                    // Add to queue
                    idxQueue.push(neighbour);
                    idxesInQueue[neighbour] = true;
                }
            }
        }
    }

    // No path found
    return false;
}

int PathfindingEngine::getGridIndex(int x, int y)
{
    return y * width + x;
}

int PathfindingEngine::calculateHeuristic(int x, int y, int destX, int destY)
{
    return std::abs(destX - x) + std::abs(destY - y);
}

int PathfindingEngine::calculateHeuristic(int idx, int destIdx)
{
    int x = idx % width;
    int y = std::floor(idx / width);
    int destX = destIdx % width;
    int destY = std::floor(destIdx / width);
    return calculateHeuristic(x, y, destX, destY);
}

std::vector<int> PathfindingEngine::getNeighbours(int idx)
{
    std::vector<int> neighbours;

    int xIndex = idx % width;

    if (xIndex > 0) neighbours.push_back(idx - 1);
    if (xIndex < width - 1) neighbours.push_back(idx + 1);
    if (idx >= width) neighbours.push_back(idx - width);
    if (idx < obstacleGrid.size() - width) neighbours.push_back(idx + width);

    return neighbours;
}

std::vector<PathfindGridCoordinate> PathfindingEngine::retracePath(int endIdx, const std::unordered_map<int, int> previousIndexes)
{
    std::vector<PathfindGridCoordinate> path;
    int currentIdx = endIdx;

    while (previousIndexes.contains(currentIdx))
    {
        PathfindGridCoordinate coordinate;
        coordinate.x = currentIdx % width;
        coordinate.y = std::floor(currentIdx / width);
        path.push_back(coordinate);

        currentIdx = previousIndexes.at(currentIdx);
    }

    return path;
}