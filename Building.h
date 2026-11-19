#ifndef BUILDING_H
#define BUILDING_H

#include <vector>
#include <string>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// Vector3 structure for 3D coordinates
struct Vector3 {
    float x, y, z;
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

// Axis-Aligned Bounding Box for walls and obstacles
struct WallBox {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
    float r, g, b; // Wall color

    WallBox() : minX(0), minY(0), minZ(0), maxX(0), maxY(0), maxZ(0), r(0.7f), g(0.7f), b(0.75f) {}
    WallBox(float x1, float y1, float z1, float x2, float y2, float z2, float _r = 0.7f, float _g = 0.7f, float _b = 0.75f)
        : minX(x1), minY(y1), minZ(z1), maxX(x2), maxY(y2), maxZ(z2), r(_r), g(_g), b(_b) {}

    bool intersects(float px, float pz, float radius) const {
        return (px + radius >= minX && px - radius <= maxX &&
                pz + radius >= minZ && pz - radius <= maxZ);
    }
};

// Emergency Exit definition
struct EmergencyExit {
    int id;
    std::string name;
    Vector3 position;
    float width;
    float depth;
    bool isBlocked;
    float signBlinkTimer;

    EmergencyExit(int _id, const std::string& _name, const Vector3& pos, float w = 3.0f, float d = 1.0f)
        : id(_id), name(_name), position(pos), width(w), depth(d), isBlocked(false), signBlinkTimer(0.0f) {}
};

// Room definition
struct Room {
    std::string name;
    float minX, minZ, maxX, maxZ;
    float floorR, floorG, floorB;

    Room(const std::string& n, float x1, float z1, float x2, float z2, float r, float g, float b)
        : name(n), minX(x1), minZ(z1), maxX(x2), maxZ(z2), floorR(r), floorG(g), floorB(b) {}
};

class Building {
public:
    Building();
    ~Building();

    void initialize();
    void update(float deltaTime);
    void render();

    // Query methods
    bool checkWallCollision(float x, float z, float radius) const;
    const std::vector<EmergencyExit>& getExits() const { return exits; }
    std::vector<EmergencyExit>& getExits() { return exits; }
    const std::vector<WallBox>& getWalls() const { return walls; }
    
    float getBuildingWidth() const { return 42.0f; }
    float getBuildingDepth() const { return 42.0f; }
    float getWallHeight() const { return wallHeight; }

private:
    float wallHeight;
    float wallThickness;
    std::vector<WallBox> walls;
    std::vector<EmergencyExit> exits;
    std::vector<Room> rooms;

    // Rendering subroutines
    void renderFloor();
    void renderWalls();
    void renderExits();
    void renderDoors();
    void renderInteriorFurniture();
    void renderPillars();
    void renderBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ, float r, float g, float b);
    void renderTexturedBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
    void renderCylinder(float x, float z, float radius, float height, float r, float g, float b);
};

#endif // BUILDING_H
