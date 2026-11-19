#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <vector>
#include <queue>
#include <cmath>
#include "Building.h"
#include "Fire.h"

// Cell type flags for navigation grid
enum GridCellType {
    CELL_FREE = 0,
    CELL_WALL = 1,
    CELL_FIRE = 2
};

// Node used internally in A* Search
struct PathNode {
    int gx, gz;
    float gCost;
    float hCost;
    float fCost() const { return gCost + hCost; }
    int parentX, parentZ;

    PathNode() : gx(0), gz(0), gCost(1e9f), hCost(0), parentX(-1), parentZ(-1) {}
    PathNode(int x, int z, float g, float h, int px = -1, int pz = -1)
        : gx(x), gz(z), gCost(g), hCost(h), parentX(px), parentZ(pz) {}

    bool operator>(const PathNode& other) const {
        return fCost() > other.fCost();
    }
};

class PathfindingGrid {
public:
    PathfindingGrid();
    ~PathfindingGrid();

    void initialize(const Building& building);
    void updateHazards(const FireManager& fireManager);

    // Coordinate conversion
    bool worldToGrid(float wx, float wz, int& gx, int& gz) const;
    void gridToWorld(int gx, int gz, float& wx, float& wz) const;
    bool isValidGrid(int gx, int gz) const;
    bool isWalkable(int gx, int gz) const;

    // A* Path calculation
    bool calculatePath(const Vector3& startPos, const Vector3& targetPos, std::vector<Vector3>& outPath);
    bool findBestExitRoute(const Vector3& startPos, const std::vector<EmergencyExit>& exits,
                           int& outExitId, std::vector<Vector3>& outPath);

    // Visualisation
    void renderPath(const std::vector<Vector3>& path, bool isTrapped = false, float personPulse = 0.0f) const;

    // Grid dimension getters
    int getWidth() const { return gridWidth; }
    int getDepth() const { return gridDepth; }
    float getCellSize() const { return cellSize; }

private:
    float minWorldX, maxWorldX;
    float minWorldZ, maxWorldZ;
    float cellSize;
    int gridWidth;
    int gridDepth;

    // 2D grid buffers
    std::vector<unsigned char> staticGrid;   // Walls & architectural boundaries
    std::vector<unsigned char> dynamicGrid;  // Static + active fires

    inline int getIndex(int gx, int gz) const {
        return gz * gridWidth + gx;
    }

    void rasterizeWall(const WallBox& wall, float margin);
    void simplifyPath(const std::vector<Vector3>& rawPath, std::vector<Vector3>& smoothedPath);
    bool hasLineOfSight(const Vector3& p1, const Vector3& p2) const;
};

#endif // PATHFINDING_H
