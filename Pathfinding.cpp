#include "Pathfinding.h"
#include <algorithm>
#include <cmath>
#include <iostream>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

PathfindingGrid::PathfindingGrid()
    : minWorldX(-21.0f), maxWorldX(21.0f),
      minWorldZ(-21.0f), maxWorldZ(21.0f),
      cellSize(0.5f) {
    gridWidth = static_cast<int>((maxWorldX - minWorldX) / cellSize);
    gridDepth = static_cast<int>((maxWorldZ - minWorldZ) / cellSize);
    staticGrid.resize(gridWidth * gridDepth, CELL_FREE);
    dynamicGrid.resize(gridWidth * gridDepth, CELL_FREE);
}

PathfindingGrid::~PathfindingGrid() {
}

bool PathfindingGrid::worldToGrid(float wx, float wz, int& gx, int& gz) const {
    if (wx < minWorldX || wx >= maxWorldX || wz < minWorldZ || wz >= maxWorldZ) {
        return false;
    }
    gx = static_cast<int>((wx - minWorldX) / cellSize);
    gz = static_cast<int>((wz - minWorldZ) / cellSize);
    if (gx < 0) gx = 0;
    if (gx >= gridWidth) gx = gridWidth - 1;
    if (gz < 0) gz = 0;
    if (gz >= gridDepth) gz = gridDepth - 1;
    return true;
}

void PathfindingGrid::gridToWorld(int gx, int gz, float& wx, float& wz) const {
    wx = minWorldX + (static_cast<float>(gx) + 0.5f) * cellSize;
    wz = minWorldZ + (static_cast<float>(gz) + 0.5f) * cellSize;
}

bool PathfindingGrid::isValidGrid(int gx, int gz) const {
    return (gx >= 0 && gx < gridWidth && gz >= 0 && gz < gridDepth);
}

bool PathfindingGrid::isWalkable(int gx, int gz) const {
    if (!isValidGrid(gx, gz)) return false;
    return dynamicGrid[getIndex(gx, gz)] == CELL_FREE;
}

void PathfindingGrid::rasterizeWall(const WallBox& wall, float margin) {
    int minGx, minGz, maxGx, maxGz;
    if (!worldToGrid(wall.minX - margin, wall.minZ - margin, minGx, minGz)) {
        minGx = std::max(0, minGx);
        minGz = std::max(0, minGz);
    }
    if (!worldToGrid(wall.maxX + margin, wall.maxZ + margin, maxGx, maxGz)) {
        maxGx = std::min(gridWidth - 1, maxGx);
        maxGz = std::min(gridDepth - 1, maxGz);
    }

    for (int gz = minGz; gz <= maxGz; ++gz) {
        for (int gx = minGx; gx <= maxGx; ++gx) {
            if (isValidGrid(gx, gz)) {
                staticGrid[getIndex(gx, gz)] = CELL_WALL;
            }
        }
    }
}

void PathfindingGrid::initialize(const Building& building) {
    std::fill(staticGrid.begin(), staticGrid.end(), CELL_FREE);

    // 1. Mark walls as solid obstacles with a radius margin for humans
    float characterMargin = 0.35f;
    const std::vector<WallBox>& walls = building.getWalls();
    for (size_t i = 0; i < walls.size(); ++i) {
        rasterizeWall(walls[i], characterMargin);
    }

    // 2. Mark building perimeter as walls, leaving emergency exit openings
    float buildingLimit = 20.0f;
    for (int gz = 0; gz < gridDepth; ++gz) {
        for (int gx = 0; gx < gridWidth; ++gx) {
            float wx, wz;
            gridToWorld(gx, gz, wx, wz);

            // Outside building boundary
            if (fabs(wx) > buildingLimit || fabs(wz) > buildingLimit) {
                // Check if this point is inside an exit corridor
                bool inExitZone = false;
                // North exit
                if (wz <= -buildingLimit && fabs(wx) <= 1.8f) inExitZone = true;
                // South exit
                if (wz >= buildingLimit && fabs(wx) <= 1.8f) inExitZone = true;
                // East exit
                if (wx >= buildingLimit && fabs(wz) <= 1.8f) inExitZone = true;
                // West exit
                if (wx <= -buildingLimit && fabs(wz) <= 1.8f) inExitZone = true;

                if (!inExitZone) {
                    staticGrid[getIndex(gx, gz)] = CELL_WALL;
                }
            }
        }
    }

    dynamicGrid = staticGrid;
}

void PathfindingGrid::updateHazards(const FireManager& fireManager) {
    // Reset dynamic grid to base architectural walls
    dynamicGrid = staticGrid;

    // Rasterize all active fire hazard danger zones
    const std::vector<FireHazard>& hazards = fireManager.getHazards();
    float safetyMargin = 0.6f;

    for (size_t i = 0; i < hazards.size(); ++i) {
        if (!hazards[i].isActive) continue;

        const FireHazard& h = hazards[i];
        float dangerDist = h.radius + safetyMargin;

        int minGx, minGz, maxGx, maxGz;
        worldToGrid(h.position.x - dangerDist, h.position.z - dangerDist, minGx, minGz);
        worldToGrid(h.position.x + dangerDist, h.position.z + dangerDist, maxGx, maxGz);

        minGx = std::max(0, minGx);
        minGz = std::max(0, minGz);
        maxGx = std::min(gridWidth - 1, maxGx);
        maxGz = std::min(gridDepth - 1, maxGz);

        for (int gz = minGz; gz <= maxGz; ++gz) {
            for (int gx = minGx; gx <= maxGx; ++gx) {
                float wx, wz;
                gridToWorld(gx, gz, wx, wz);
                float dx = wx - h.position.x;
                float dz = wz - h.position.z;
                if ((dx * dx + dz * dz) <= (dangerDist * dangerDist)) {
                    dynamicGrid[getIndex(gx, gz)] = CELL_FIRE;
                }
            }
        }
    }
}

// Check line of sight using Bresenham's ray marching
bool PathfindingGrid::hasLineOfSight(const Vector3& p1, const Vector3& p2) const {
    float dist = sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.z - p1.z) * (p2.z - p1.z));
    int steps = static_cast<int>(dist / (cellSize * 0.5f)) + 1;
    for (int i = 0; i <= steps; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        float wx = p1.x + t * (p2.x - p1.x);
        float wz = p1.z + t * (p2.z - p1.z);
        int gx, gz;
        if (!worldToGrid(wx, wz, gx, gz) || !isWalkable(gx, gz)) {
            return false;
        }
    }
    return true;
}

void PathfindingGrid::simplifyPath(const std::vector<Vector3>& rawPath, std::vector<Vector3>& smoothedPath) {
    if (rawPath.size() <= 2) {
        smoothedPath = rawPath;
        return;
    }

    smoothedPath.clear();
    smoothedPath.push_back(rawPath[0]);

    size_t currentIdx = 0;
    while (currentIdx < rawPath.size() - 1) {
        size_t furthestIdx = currentIdx + 1;
        // Look ahead as far as possible with clear line of sight
        for (size_t nextIdx = rawPath.size() - 1; nextIdx > currentIdx + 1; --nextIdx) {
            if (hasLineOfSight(rawPath[currentIdx], rawPath[nextIdx])) {
                furthestIdx = nextIdx;
                break;
            }
        }
        smoothedPath.push_back(rawPath[furthestIdx]);
        currentIdx = furthestIdx;
    }
}

bool PathfindingGrid::calculatePath(const Vector3& startPos, const Vector3& targetPos, std::vector<Vector3>& outPath) {
    outPath.clear();

    int startGx, startGz, targetGx, targetGz;
    if (!worldToGrid(startPos.x, startPos.z, startGx, startGz) ||
        !worldToGrid(targetPos.x, targetPos.z, targetGx, targetGz)) {
        return false;
    }

    // If start cell is blocked, find closest walkable cell within 2 cells radius
    if (!isWalkable(startGx, startGz)) {
        bool found = false;
        for (int r = 1; r <= 3 && !found; ++r) {
            for (int dz = -r; dz <= r && !found; ++dz) {
                for (int dx = -r; dx <= r && !found; ++dx) {
                    int nx = startGx + dx;
                    int nz = startGz + dz;
                    if (isWalkable(nx, nz)) {
                        startGx = nx;
                        startGz = nz;
                        found = true;
                    }
                }
            }
        }
        if (!found) return false;
    }

    // If target cell is blocked, find closest walkable cell
    if (!isWalkable(targetGx, targetGz)) {
        bool found = false;
        for (int r = 1; r <= 4 && !found; ++r) {
            for (int dz = -r; dz <= r && !found; ++dz) {
                for (int dx = -r; dx <= r && !found; ++dx) {
                    int nx = targetGx + dx;
                    int nz = targetGz + dz;
                    if (isWalkable(nx, nz)) {
                        targetGx = nx;
                        targetGz = nz;
                        found = true;
                    }
                }
            }
        }
        if (!found) return false;
    }

    // A* Priority Queue
    std::priority_queue<PathNode, std::vector<PathNode>, std::greater<PathNode> > openList;
    std::vector<float> gCostBuffer(gridWidth * gridDepth, 1e9f);
    std::vector<int> parentXBuffer(gridWidth * gridDepth, -1);
    std::vector<int> parentZBuffer(gridWidth * gridDepth, -1);
    std::vector<bool> closedList(gridWidth * gridDepth, false);

    // Heuristic: Euclidean distance
    auto calcH = [targetGx, targetGz](int x, int z) -> float {
        float dx = static_cast<float>(x - targetGx);
        float dz = static_cast<float>(z - targetGz);
        return sqrt(dx * dx + dz * dz);
    };

    int startIndex = getIndex(startGx, startGz);
    gCostBuffer[startIndex] = 0.0f;
    openList.push(PathNode(startGx, startGz, 0.0f, calcH(startGx, startGz), -1, -1));

    // 8-directional movement offsets
    const int dirX[8] = { 0, 1, 0, -1, 1, 1, -1, -1 };
    const int dirZ[8] = { -1, 0, 1, 0, -1, 1, 1, -1 };
    const float dirCost[8] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.4142f, 1.4142f, 1.4142f, 1.4142f };

    bool targetReached = false;

    while (!openList.empty()) {
        PathNode current = openList.top();
        openList.pop();

        int curIdx = getIndex(current.gx, current.gz);
        if (closedList[curIdx]) continue;
        closedList[curIdx] = true;

        if (current.gx == targetGx && current.gz == targetGz) {
            targetReached = true;
            break;
        }

        // Explore neighbors
        for (int i = 0; i < 8; ++i) {
            int nx = current.gx + dirX[i];
            int nz = current.gz + dirZ[i];

            if (!isValidGrid(nx, nz) || !isWalkable(nx, nz)) continue;

            // Corner-cutting check for diagonal movements:
            // Ensure both orthogonal adjacent cells are free!
            if (i >= 4) {
                int adj1X = current.gx + dirX[i];
                int adj1Z = current.gz;
                int adj2X = current.gx;
                int adj2Z = current.gz + dirZ[i];
                if (!isWalkable(adj1X, adj1Z) || !isWalkable(adj2X, adj2Z)) {
                    continue; // Disallow cutting through wall corner
                }
            }

            int nextIdx = getIndex(nx, nz);
            if (closedList[nextIdx]) continue;

            float newGCost = current.gCost + dirCost[i];
            if (newGCost < gCostBuffer[nextIdx]) {
                gCostBuffer[nextIdx] = newGCost;
                parentXBuffer[nextIdx] = current.gx;
                parentZBuffer[nextIdx] = current.gz;
                openList.push(PathNode(nx, nz, newGCost, calcH(nx, nz), current.gx, current.gz));
            }
        }
    }

    if (!targetReached) {
        return false; // No safe path found
    }

    // Reconstruct raw path from target to start
    std::vector<Vector3> rawPath;
    int currX = targetGx;
    int currZ = targetGz;

    while (currX != -1 && currZ != -1) {
        float wx, wz;
        gridToWorld(currX, currZ, wx, wz);
        rawPath.push_back(Vector3(wx, 0.05f, wz));

        if (currX == startGx && currZ == startGz) break;

        int idx = getIndex(currX, currZ);
        int px = parentXBuffer[idx];
        int pz = parentZBuffer[idx];
        currX = px;
        currZ = pz;
    }

    std::reverse(rawPath.begin(), rawPath.end());

    // Smooth path by string-pulling line of sight
    simplifyPath(rawPath, outPath);

    // Append exact target position at the end
    if (!outPath.empty()) {
        outPath[outPath.size() - 1] = targetPos;
    }

    return true;
}

bool PathfindingGrid::findBestExitRoute(const Vector3& startPos, const std::vector<EmergencyExit>& exits,
                                       int& outExitId, std::vector<Vector3>& outPath) {
    outPath.clear();
    outExitId = -1;

    float bestDistance = 1e9f;
    std::vector<Vector3> bestCandidatePath;
    int bestExit = -1;

    for (size_t i = 0; i < exits.size(); ++i) {
        if (exits[i].isBlocked) continue;

        std::vector<Vector3> candidatePath;
        if (calculatePath(startPos, exits[i].position, candidatePath)) {
            // Compute total geometric path length
            float pathLen = 0.0f;
            for (size_t j = 1; j < candidatePath.size(); ++j) {
                float dx = candidatePath[j].x - candidatePath[j - 1].x;
                float dz = candidatePath[j].z - candidatePath[j - 1].z;
                pathLen += sqrt(dx * dx + dz * dz);
            }

            if (pathLen < bestDistance) {
                bestDistance = pathLen;
                bestCandidatePath = candidatePath;
                bestExit = exits[i].id;
            }
        }
    }

    if (bestExit != -1) {
        outExitId = bestExit;
        outPath = bestCandidatePath;
        return true;
    }

    return false; // All exits are trapped or unreachable
}

void PathfindingGrid::renderPath(const std::vector<Vector3>& path, bool isTrapped, float personPulse) const {
    if (path.empty()) return;

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive luminous green glow

    if (isTrapped) {
        // Trapped route: flashing warning red
        float flash = 0.5f + 0.5f * sin(personPulse * 8.0f);
        glColor4f(1.0f * flash, 0.1f, 0.1f, 0.85f);
        glLineWidth(3.0f);
    } else {
        // Safe evacuation route: vibrant bright neon green
        glColor4f(0.1f, 1.0f, 0.35f, 0.85f);
        glLineWidth(2.5f);
    }

    glBegin(GL_LINE_STRIP);
    for (size_t i = 0; i < path.size(); ++i) {
        glVertex3f(path[i].x, path[i].y + 0.08f, path[i].z);
    }
    glEnd();

    // Render luminous waypoint beads along the path
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < path.size(); ++i) {
        glVertex3f(path[i].x, path[i].y + 0.08f, path[i].z);
    }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}
