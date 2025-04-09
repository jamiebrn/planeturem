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

bool PathfindingEngine::findPath(int startX, int startY, int endX, int endY, std::vector<PathfindGridCoordinate>& result, bool straightening,
    std::optional<int> maxDistance) const
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

        int nextIdx = idx - 1;
        if (xIndex - 1 < 0)
        {
            nextIdx += width;
        }
        advancePathNode(nextIdx, idx, previousNode.pathCost, endIdx, 3, previousNode.direction, pathNodes, idxQueue, straightening, maxDistance);

        nextIdx = idx + 1;
        if (xIndex + 1 > width - 1)
        {
            nextIdx -= width;
        }
        advancePathNode(nextIdx, idx, previousNode.pathCost, endIdx, 1, previousNode.direction, pathNodes, idxQueue, straightening, maxDistance);

        nextIdx = idx - width;
        if (nextIdx < 0)
        {
            nextIdx += obstacleGrid.size();
        }
        advancePathNode(nextIdx, idx, previousNode.pathCost, endIdx, 0, previousNode.direction, pathNodes, idxQueue, straightening, maxDistance);

        nextIdx = idx + width;
        if (nextIdx >= obstacleGrid.size())
        {
            nextIdx -= obstacleGrid.size();
        }
        advancePathNode(nextIdx, idx, previousNode.pathCost, endIdx, 2, previousNode.direction, pathNodes, idxQueue, straightening, maxDistance);
    }

    // No path found
    return false;
}

int PathfindingEngine::getGridIndex(int x, int y) const
{
    return y * width + x;
}

PathfindGridCoordinate PathfindingEngine::getGridCoordinate(int gridIndex) const
{
    PathfindGridCoordinate coordinate;
    coordinate.x = gridIndex % width;
    coordinate.y = std::floor(gridIndex / width);
    return coordinate;
}

int PathfindingEngine::calculateHeuristic(int x, int y, int destX, int destY) const
{
    int dx = std::abs(destX - x);
    int dy = std::abs(destY - y);

    int wrappedDx = std::min(dx, width - dx);
    int wrappedDy = std::min(dy, height - dy);

    return wrappedDx + wrappedDy;
}

int PathfindingEngine::calculateHeuristic(int idx, int destIdx) const
{
    int x = idx % width;
    int y = idx / width;
    int destX = destIdx % width;
    int destY = destIdx / width;
    return calculateHeuristic(x, y, destX, destY);
}

void PathfindingEngine::advancePathNode(int idx, int previousIdx, int previousPathCost, int destIdx, int direction, int previousDirection,
    std::unordered_map<int, PathNode>& pathNodes, std::priority_queue<int, std::vector<int>, PathNodeComparator>& idxQueue, bool straightening,
    std::optional<int> maxDistance) const
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

    if (maxDistance.has_value())
    {
        if (newCost >= maxDistance.value())
        {
            return;
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

std::vector<PathfindGridCoordinate> PathfindingEngine::createStepSequenceFromPath(const std::vector<PathfindGridCoordinate>& path) const
{
    std::vector<PathfindGridCoordinate> steps;
    for (int i = path.size() - 1; i >= 1; i--)
    {
        PathfindGridCoordinate current = path[i];
        PathfindGridCoordinate next = path[i - 1];

        int dx = next.x - current.x;
        int dy = next.y - current.y;

        if (dx >= width - 1)
        {
            dx = -1;
        }
        else if (dx <= -width + 1)
        {
            dx = 1;
        }
        if (dy >= height - 1)
        {
            dy = -1;
        }
        else if (dy <= -width + 1)
        {
            dy = 1;
        }

        PathfindGridCoordinate step;
        step.x = dx;
        step.y = dy;

        steps.push_back(step);
    }

    return steps;
}

PathfindGridCoordinate PathfindingEngine::findFurthestOpenTile(int x, int y, int maxSearchRange, bool coordinateRelativeToStart) const
{
    struct TileSearchNode
    {
        int index;
        int distanceTravelled;
    };

    int startIdx = getGridIndex(x, y);

    std::queue<TileSearchNode> indexQueue;
    std::unordered_map<int, bool> visitedIndexes;
    indexQueue.push(TileSearchNode{startIdx, 0});

    int furthestPointIndex = startIdx;
    int furthestPointDistance = -1;

    while (!indexQueue.empty())
    {
        TileSearchNode node = indexQueue.front();
        indexQueue.pop();

        if (obstacleGrid[node.index] || maxSearchRange < node.distanceTravelled || visitedIndexes[node.index])
        {
            continue;
        }

        visitedIndexes[node.index] = true;

        if (furthestPointDistance < node.distanceTravelled)
        {
            furthestPointIndex = node.index;
            furthestPointDistance = node.distanceTravelled;
        }

        int xIndex = node.index % width;

        int nextIdx = node.index - 1;
        if (xIndex - 1 < 0)
        {
            nextIdx += width;
        }
        indexQueue.push(TileSearchNode{nextIdx, node.distanceTravelled + 1});

        nextIdx = node.index + 1;
        if (xIndex + 1 > width - 1)
        {
            nextIdx -= width;
        }
        indexQueue.push(TileSearchNode{nextIdx, node.distanceTravelled + 1});

        nextIdx = node.index - width;
        if (nextIdx < 0)
        {
            nextIdx += obstacleGrid.size();
        }
        indexQueue.push(TileSearchNode{nextIdx, node.distanceTravelled + 1});

        nextIdx = node.index + width;
        if (nextIdx >= obstacleGrid.size())
        {
            nextIdx -= obstacleGrid.size();
        }
        indexQueue.push(TileSearchNode{nextIdx, node.distanceTravelled + 1});   
    }

    PathfindGridCoordinate gridCoord = getGridCoordinate(furthestPointIndex);

    if (coordinateRelativeToStart)
    {
        int dx = gridCoord.x - x;
        int dy = gridCoord.y - y;

        if (std::abs(dx) > width / 2)
        {
            dx = (std::abs(dx) - width) * Helper::sign(dx);
        }
        if (std::abs(dy) > height / 2)
        {
            dy = (std::abs(dy) - height) * Helper::sign(dy);
        }

        gridCoord.x = dx;
        gridCoord.y = dy;
    }

    return gridCoord;
}

void PathFollower::beginPath(pl::Vector2f startPos, const std::vector<PathfindGridCoordinate>& pathfindStepSequence)
{
    stepSequence = pathfindStepSequence;
    lastStepPosition = startPos;
    position = lastStepPosition;
    setPathfindStepIndex(0);
}

pl::Vector2f PathFollower::updateFollower(float speed)
{
    if (isActive())
    {
        position += Helper::normaliseVector(stepTargetPosition - position) * speed;

        if (Helper::getVectorLength(stepTargetPosition - position) <= TARGET_REACH_THRESHOLD)
        {
            // Move to next pathfinding step
            lastStepPosition = stepTargetPosition;
            setPathfindStepIndex(stepIndex + 1);
        }
    }

    return position;
}

bool PathFollower::isActive()
{
    return (stepIndex < stepSequence.size());
}

void PathFollower::handleWorldWrap(pl::Vector2f positionDelta)
{
    position += positionDelta;
    lastStepPosition += positionDelta;
    stepTargetPosition += positionDelta;
}

void PathFollower::setPathfindStepIndex(int index)
{
    stepIndex = index;

    if (stepIndex >= stepSequence.size())
    {
        return;
    }

    PathfindGridCoordinate target = stepSequence[stepIndex];
    pl::Vector2<int> tilePosition = WorldObject::getTileInside(lastStepPosition);
    stepTargetPosition = pl::Vector2f(target.x + tilePosition.x + 0.5f, target.y + tilePosition.y + 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
}