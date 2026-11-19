#include "Building.h"
#include <cmath>
#include <cstring>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Building::Building() : wallHeight(2.6f), wallThickness(0.4f) {
}

Building::~Building() {
}

void Building::initialize() {
    walls.clear();
    exits.clear();
    rooms.clear();

    // 1. Define Rooms
    // Office A (NW)
    rooms.push_back(Room("OFFICE A", -19.5f, -19.5f, -2.5f, -2.5f, 0.22f, 0.25f, 0.30f));
    // Server Lab B (NE)
    rooms.push_back(Room("SERVER LAB B", 2.5f, -19.5f, 19.5f, -2.5f, 0.20f, 0.22f, 0.28f));
    // Cafeteria C (SW)
    rooms.push_back(Room("CAFETERIA C", -19.5f, 2.5f, -2.5f, 19.5f, 0.28f, 0.24f, 0.20f));
    // Research Lab D (SE)
    rooms.push_back(Room("RESEARCH D", 2.5f, 2.5f, 19.5f, 19.5f, 0.22f, 0.28f, 0.25f));

    // 2. Define Emergency Exits (4 major exits at corridor ends)
    // North Exit (Exit 1)
    exits.push_back(EmergencyExit(1, "NORTH EXIT", Vector3(0.0f, 0.0f, -20.0f), 3.6f, 0.8f));
    // South Exit (Exit 2)
    exits.push_back(EmergencyExit(2, "SOUTH EXIT", Vector3(0.0f, 0.0f, 20.0f), 3.6f, 0.8f));
    // East Exit (Exit 3)
    exits.push_back(EmergencyExit(3, "EAST EXIT", Vector3(20.0f, 0.0f, 0.0f), 0.8f, 3.6f));
    // West Exit (Exit 4)
    exits.push_back(EmergencyExit(4, "WEST EXIT", Vector3(-20.0f, 0.0f, 0.0f), 0.8f, 3.6f));

    // 3. Define Walls (Solid extruded boxes with height = wallHeight)
    // Wall colors
    float outerR = 0.45f, outerG = 0.48f, outerB = 0.55f;
    float innerR = 0.55f, innerG = 0.58f, innerB = 0.65f;
    float colR = 0.35f, colG = 0.38f, colB = 0.42f;

    // --- OUTER PERIMETER WALLS (with Exit Openings) ---
    // North outer wall: [-20, -2] and [2, 20] (Leaving opening [-2, 2] for North Exit)
    walls.push_back(WallBox(-20.2f, 0.0f, -20.2f, -1.8f, wallHeight, -19.8f, outerR, outerG, outerB));
    walls.push_back(WallBox(1.8f, 0.0f, -20.2f, 20.2f, wallHeight, -19.8f, outerR, outerG, outerB));

    // South outer wall: [-20, -2] and [2, 20] (Leaving opening [-2, 2] for South Exit)
    walls.push_back(WallBox(-20.2f, 0.0f, 19.8f, -1.8f, wallHeight, 20.2f, outerR, outerG, outerB));
    walls.push_back(WallBox(1.8f, 0.0f, 19.8f, 20.2f, wallHeight, 20.2f, outerR, outerG, outerB));

    // West outer wall: [-20.2, -19.8], from Z = -20 to -2 and Z = 2 to 20 (Exit at Z = [-2, 2])
    walls.push_back(WallBox(-20.2f, 0.0f, -20.0f, -19.8f, wallHeight, -1.8f, outerR, outerG, outerB));
    walls.push_back(WallBox(-20.2f, 0.0f, 1.8f, -19.8f, wallHeight, 20.0f, outerR, outerG, outerB));

    // East outer wall: [19.8, 20.2], from Z = -20 to -2 and Z = 2 to 20 (Exit at Z = [-2, 2])
    walls.push_back(WallBox(19.8f, 0.0f, -20.0f, 20.2f, wallHeight, -1.8f, outerR, outerG, outerB));
    walls.push_back(WallBox(19.8f, 0.0f, 1.8f, 20.2f, wallHeight, 20.0f, outerR, outerG, outerB));

    // --- INTERIOR ROOM WALLS (with doorway openings) ---

    // NW Room (Office A):
    // Wall facing central corridor (at X = -2.5f, from Z = -19.8 to -2.3): Doorway at Z in [-12.0, -9.0]
    walls.push_back(WallBox(-2.7f, 0.0f, -19.8f, -2.3f, wallHeight, -12.0f, innerR, innerG, innerB));
    walls.push_back(WallBox(-2.7f, 0.0f, -9.0f, -2.3f, wallHeight, -2.3f, innerR, innerG, innerB));
    // Wall facing cross corridor (at Z = -2.5f, from X = -19.8 to -2.3): Doorway at X in [-12.0, -9.0]
    walls.push_back(WallBox(-19.8f, 0.0f, -2.7f, -12.0f, wallHeight, -2.3f, innerR, innerG, innerB));
    walls.push_back(WallBox(-9.0f, 0.0f, -2.7f, -2.3f, wallHeight, -2.3f, innerR, innerG, innerB));

    // NE Room (Server Lab B):
    // Wall facing central corridor (at X = 2.5f, from Z = -19.8 to -2.3): Doorway at Z in [-12.0, -9.0]
    walls.push_back(WallBox(2.3f, 0.0f, -19.8f, 2.7f, wallHeight, -12.0f, innerR, innerG, innerB));
    walls.push_back(WallBox(2.3f, 0.0f, -9.0f, 2.7f, wallHeight, -2.3f, innerR, innerG, innerB));
    // Wall facing cross corridor (at Z = -2.5f, from X = 2.3 to 19.8): Doorway at X in [9.0, 12.0]
    walls.push_back(WallBox(2.3f, 0.0f, -2.7f, 9.0f, wallHeight, -2.3f, innerR, innerG, innerB));
    walls.push_back(WallBox(12.0f, 0.0f, -2.7f, 19.8f, wallHeight, -2.3f, innerR, innerG, innerB));

    // SW Room (Cafeteria C):
    // Wall facing central corridor (at X = -2.5f, from Z = 2.3 to 19.8): Doorway at Z in [9.0, 12.0]
    walls.push_back(WallBox(-2.7f, 0.0f, 2.3f, -2.3f, wallHeight, 9.0f, innerR, innerG, innerB));
    walls.push_back(WallBox(-2.7f, 0.0f, 12.0f, -2.3f, wallHeight, 19.8f, innerR, innerG, innerB));
    // Wall facing cross corridor (at Z = 2.5f, from X = -19.8 to -2.3): Doorway at X in [-12.0, -9.0]
    walls.push_back(WallBox(-19.8f, 0.0f, 2.3f, -12.0f, wallHeight, 2.7f, innerR, innerG, innerB));
    walls.push_back(WallBox(-9.0f, 0.0f, 2.3f, -2.3f, wallHeight, 2.7f, innerR, innerG, innerB));

    // SE Room (Research D):
    // Wall facing central corridor (at X = 2.5f, from Z = 2.3 to 19.8): Doorway at Z in [9.0, 12.0]
    walls.push_back(WallBox(2.3f, 0.0f, 2.3f, 2.7f, wallHeight, 9.0f, innerR, innerG, innerB));
    walls.push_back(WallBox(2.3f, 0.0f, 12.0f, 2.7f, wallHeight, 19.8f, innerR, innerG, innerB));
    // Wall facing cross corridor (at Z = 2.5f, from X = 2.3 to 19.8): Doorway at X in [9.0, 12.0]
    walls.push_back(WallBox(2.3f, 0.0f, 2.3f, 9.0f, wallHeight, 2.7f, innerR, innerG, innerB));
    walls.push_back(WallBox(12.0f, 0.0f, 2.3f, 19.8f, wallHeight, 2.7f, innerR, innerG, innerB));

    // Internal Room Dividers for realistic layout:
    // Office A partition (creates manager cubicle)
    walls.push_back(WallBox(-15.0f, 0.0f, -14.0f, -9.0f, 1.8f, -13.7f, 0.6f, 0.55f, 0.45f));
    // Server Lab B partition (server cage)
    walls.push_back(WallBox(9.0f, 0.0f, -14.0f, 15.0f, 2.2f, -13.7f, 0.35f, 0.40f, 0.45f));
    // Cafeteria C partition (serving counter)
    walls.push_back(WallBox(-15.0f, 0.0f, 10.0f, -9.0f, 1.2f, 10.5f, 0.55f, 0.45f, 0.35f));
    // Research D partition (chemical fume hood/divider)
    walls.push_back(WallBox(9.0f, 0.0f, 10.0f, 15.0f, 2.0f, 10.4f, 0.45f, 0.50f, 0.55f));

    // Structural Columns at Corridor Crossway
    float colSize = 0.5f;
    walls.push_back(WallBox(-2.5f - colSize, 0.0f, -2.5f - colSize, -2.5f, wallHeight, -2.5f, colR, colG, colB));
    walls.push_back(WallBox(2.5f, 0.0f, -2.5f - colSize, 2.5f + colSize, wallHeight, -2.5f, colR, colG, colB));
    walls.push_back(WallBox(-2.5f - colSize, 0.0f, 2.5f, -2.5f, wallHeight, 2.5f + colSize, colR, colG, colB));
    walls.push_back(WallBox(2.5f, 0.0f, 2.5f, 2.5f + colSize, wallHeight, 2.5f + colSize, colR, colG, colB));
}

void Building::update(float deltaTime) {
    for (size_t i = 0; i < exits.size(); ++i) {
        exits[i].signBlinkTimer += deltaTime;
    }
}

bool Building::checkWallCollision(float x, float z, float radius) const {
    // Check against all walls and obstacles
    for (size_t i = 0; i < walls.size(); ++i) {
        if (walls[i].intersects(x, z, radius)) {
            return true;
        }
    }
    // Check building outer bounds (except at exits)
    if (x - radius < -20.0f) {
        if (fabs(z) > 1.8f) return true; // Blocked outside exit zone
        if (x < -21.5f) return true;
    }
    if (x + radius > 20.0f) {
        if (fabs(z) > 1.8f) return true;
        if (x > 21.5f) return true;
    }
    if (z - radius < -20.0f) {
        if (fabs(x) > 1.8f) return true;
        if (z < -21.5f) return true;
    }
    if (z + radius > 20.0f) {
        if (fabs(x) > 1.8f) return true;
        if (z > 21.5f) return true;
    }
    return false;
}

void Building::render() {
    renderFloor();
    renderWalls();
    renderExits();
    renderDoors();
    renderInteriorFurniture();
}

void Building::renderBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ, float r, float g, float b) {
    GLfloat mat_ambient[] = { r * 0.3f, g * 0.3f, b * 0.3f, 1.0f };
    GLfloat mat_diffuse[] = { r, g, b, 1.0f };
    GLfloat mat_specular[] = { 0.15f, 0.15f, 0.15f, 1.0f };
    GLfloat mat_shininess[] = { 10.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glBegin(GL_QUADS);
    // Front face (Z = maxZ)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(minX, minY, maxZ);
    glVertex3f(maxX, minY, maxZ);
    glVertex3f(maxX, maxY, maxZ);
    glVertex3f(minX, maxY, maxZ);

    // Back face (Z = minZ)
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(minX, minY, minZ);
    glVertex3f(minX, maxY, minZ);
    glVertex3f(maxX, maxY, minZ);
    glVertex3f(maxX, minY, minZ);

    // Top face (Y = maxY)
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(minX, maxY, minZ);
    glVertex3f(minX, maxY, maxZ);
    glVertex3f(maxX, maxY, maxZ);
    glVertex3f(maxX, maxY, minZ);

    // Bottom face (Y = minY)
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(minX, minY, minZ);
    glVertex3f(maxX, minY, minZ);
    glVertex3f(maxX, minY, maxZ);
    glVertex3f(minX, minY, maxZ);

    // Right face (X = maxX)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(maxX, minY, minZ);
    glVertex3f(maxX, maxY, minZ);
    glVertex3f(maxX, maxY, maxZ);
    glVertex3f(maxX, minY, maxZ);

    // Left face (X = minX)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(minX, minY, minZ);
    glVertex3f(minX, minY, maxZ);
    glVertex3f(minX, maxY, maxZ);
    glVertex3f(minX, maxY, minZ);
    glEnd();
}

void Building::renderFloor() {
    // 1. Exterior Ground Plate (Dark slate outside the building)
    renderBox(-28.0f, -0.3f, -28.0f, 28.0f, -0.05f, 28.0f, 0.08f, 0.09f, 0.12f);

    // 2. Corridors (Lighter polished gray floor with tile borders)
    // Central Corridor (Z-axis)
    renderBox(-2.4f, -0.04f, -20.0f, 2.4f, 0.0f, 20.0f, 0.32f, 0.34f, 0.38f);
    // Cross Corridor (X-axis)
    renderBox(-20.0f, -0.04f, -2.4f, 20.0f, 0.0f, 2.4f, 0.32f, 0.34f, 0.38f);

    // Corridor floor striping / evacuation guide lines (luminous cyan dashes)
    glDisable(GL_LIGHTING);
    glColor3f(0.18f, 0.45f, 0.55f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    for (float z = -19.0f; z <= 19.0f; z += 2.0f) {
        glVertex3f(0.0f, 0.01f, z);
        glVertex3f(0.0f, 0.01f, z + 1.0f);
    }
    for (float x = -19.0f; x <= 19.0f; x += 2.0f) {
        glVertex3f(x, 0.01f, 0.0f);
        glVertex3f(x + 1.0f, 0.01f, 0.0f);
    }
    glEnd();
    glEnable(GL_LIGHTING);

    // 3. Room Floors with distinctive colors and subtle borders
    for (size_t i = 0; i < rooms.size(); ++i) {
        const Room& r = rooms[i];
        renderBox(r.minX, -0.04f, r.minZ, r.maxX, 0.0f, r.maxZ, r.floorR, r.floorG, r.floorB);
    }
}

void Building::renderWalls() {
    // Render all walls
    for (size_t i = 0; i < walls.size(); ++i) {
        const WallBox& w = walls[i];
        renderBox(w.minX, w.minY, w.minZ, w.maxX, w.maxY, w.maxZ, w.r, w.g, w.b);

        // Render stylish dark wall capping / top trim
        renderBox(w.minX - 0.02f, w.maxY - 0.05f, w.minZ - 0.02f,
                  w.maxX + 0.02f, w.maxY, w.maxZ + 0.02f,
                  0.18f, 0.20f, 0.25f);
    }
}

void Building::renderExits() {
    for (size_t i = 0; i < exits.size(); ++i) {
        const EmergencyExit& ex = exits[i];
        float px = ex.position.x;
        float pz = ex.position.z;

        // Exit Gate Floor Marker (Glowing green pad)
        float pulse = 0.7f + 0.3f * sin(ex.signBlinkTimer * 4.0f);
        if (ex.isBlocked) {
            renderBox(px - 1.8f, 0.01f, pz - 0.5f, px + 1.8f, 0.02f, pz + 0.5f, 0.8f * pulse, 0.1f, 0.1f);
        } else {
            renderBox(px - 1.8f, 0.01f, pz - 0.5f, px + 1.8f, 0.02f, pz + 0.5f, 0.1f, 0.9f * pulse, 0.2f);
        }

        // Exit Archway Posts (Left and Right pillars)
        float postColorR = ex.isBlocked ? 0.7f : 0.15f;
        float postColorG = ex.isBlocked ? 0.15f : 0.85f;
        float postColorB = 0.15f;

        if (fabs(pz) > fabs(px)) {
            // North or South Exit
            renderBox(px - 1.8f, 0.0f, pz - 0.2f, px - 1.6f, wallHeight, pz + 0.2f, postColorR, postColorG, postColorB);
            renderBox(px + 1.6f, 0.0f, pz - 0.2f, px + 1.8f, wallHeight, pz + 0.2f, postColorR, postColorG, postColorB);
            // Top lintel / overhead arch beam
            renderBox(px - 1.8f, wallHeight - 0.3f, pz - 0.2f, px + 1.8f, wallHeight, pz + 0.2f, postColorR, postColorG, postColorB);

            // Illuminated "EXIT" Sign Box
            renderBox(px - 0.9f, wallHeight - 0.55f, pz - 0.25f, px + 0.9f, wallHeight - 0.28f, pz + 0.25f, 0.05f, 0.05f, 0.05f);
            // Front & Back Green Glow panels
            glDisable(GL_LIGHTING);
            if (ex.isBlocked) {
                glColor3f(1.0f * pulse, 0.1f, 0.1f);
            } else {
                glColor3f(0.1f, 1.0f * pulse, 0.2f);
            }
            glBegin(GL_QUADS);
            // Toward inside
            float signZ = (pz < 0) ? (pz + 0.26f) : (pz - 0.26f);
            glVertex3f(px - 0.85f, wallHeight - 0.52f, signZ);
            glVertex3f(px + 0.85f, wallHeight - 0.52f, signZ);
            glVertex3f(px + 0.85f, wallHeight - 0.31f, signZ);
            glVertex3f(px - 0.85f, wallHeight - 0.31f, signZ);
            glEnd();
            glEnable(GL_LIGHTING);
        } else {
            // East or West Exit
            renderBox(px - 0.2f, 0.0f, pz - 1.8f, px + 0.2f, wallHeight, pz - 1.6f, postColorR, postColorG, postColorB);
            renderBox(px - 0.2f, 0.0f, pz + 1.6f, px + 0.2f, wallHeight, pz + 1.8f, postColorR, postColorG, postColorB);
            // Top lintel
            renderBox(px - 0.2f, wallHeight - 0.3f, pz - 1.8f, px + 0.2f, wallHeight, pz + 1.8f, postColorR, postColorG, postColorB);

            // Illuminated Sign Box
            renderBox(px - 0.25f, wallHeight - 0.55f, pz - 0.9f, px + 0.25f, wallHeight - 0.28f, pz + 0.9f, 0.05f, 0.05f, 0.05f);
            // Glow panels
            glDisable(GL_LIGHTING);
            if (ex.isBlocked) {
                glColor3f(1.0f * pulse, 0.1f, 0.1f);
            } else {
                glColor3f(0.1f, 1.0f * pulse, 0.2f);
            }
            glBegin(GL_QUADS);
            float signX = (px < 0) ? (px + 0.26f) : (px - 0.26f);
            glVertex3f(signX, wallHeight - 0.52f, pz - 0.85f);
            glVertex3f(signX, wallHeight - 0.52f, pz + 0.85f);
            glVertex3f(signX, wallHeight - 0.31f, pz + 0.85f);
            glVertex3f(signX, wallHeight - 0.31f, pz - 0.85f);
            glEnd();
            glEnable(GL_LIGHTING);
        }
    }
}

void Building::renderDoors() {
    // Render top lintel overhead framing for doorways
    float frameR = 0.30f, frameG = 0.32f, frameB = 0.38f;

    // NW Room doors (at Z = -10.5 and X = -10.5)
    renderBox(-2.7f, wallHeight - 0.4f, -12.0f, -2.3f, wallHeight, -9.0f, frameR, frameG, frameB);
    renderBox(-12.0f, wallHeight - 0.4f, -2.7f, -9.0f, wallHeight, -2.3f, frameR, frameG, frameB);

    // NE Room doors
    renderBox(2.3f, wallHeight - 0.4f, -12.0f, 2.7f, wallHeight, -9.0f, frameR, frameG, frameB);
    renderBox(9.0f, wallHeight - 0.4f, -2.7f, 12.0f, wallHeight, -2.3f, frameR, frameG, frameB);

    // SW Room doors
    renderBox(-2.7f, wallHeight - 0.4f, 9.0f, -2.3f, wallHeight, 12.0f, frameR, frameG, frameB);
    renderBox(-12.0f, wallHeight - 0.4f, 2.3f, -9.0f, wallHeight, 2.7f, frameR, frameG, frameB);

    // SE Room doors
    renderBox(2.3f, wallHeight - 0.4f, 9.0f, 2.7f, wallHeight, 12.0f, frameR, frameG, frameB);
    renderBox(9.0f, wallHeight - 0.4f, 2.3f, 12.0f, wallHeight, 2.7f, frameR, frameG, frameB);
}

void Building::renderInteriorFurniture() {
    // Office A: Large Conference Table and Chairs
    renderBox(-12.5f, 0.0f, -12.0f, -7.5f, 0.75f, -8.0f, 0.45f, 0.32f, 0.20f); // Table top
    renderBox(-12.3f, 0.0f, -11.8f, -11.9f, 0.70f, -11.4f, 0.15f, 0.15f, 0.15f); // Table leg
    renderBox(-8.1f, 0.0f, -8.6f, -7.7f, 0.70f, -8.2f, 0.15f, 0.15f, 0.15f); // Table leg

    // Server Lab B: Multiple Server Racks with LED indicators
    for (float z = -17.0f; z <= -6.0f; z += 2.2f) {
        renderBox(14.0f, 0.0f, z - 0.8f, 16.5f, 2.2f, z + 0.8f, 0.12f, 0.14f, 0.18f); // Rack chassis
        // Glowing blue/green server status LEDs
        glDisable(GL_LIGHTING);
        glColor3f(0.1f, 0.8f, 0.9f);
        glPointSize(3.0f);
        glBegin(GL_POINTS);
        for (float y = 0.4f; y <= 2.0f; y += 0.3f) {
            glVertex3f(13.98f, y, z);
        }
        glEnd();
        glEnable(GL_LIGHTING);
    }

    // Cafeteria C: Dining tables
    for (float x = -16.0f; x <= -6.0f; x += 4.5f) {
        for (float z = 6.0f; z <= 16.0f; z += 4.5f) {
            renderBox(x - 0.9f, 0.0f, z - 0.9f, x + 0.9f, 0.7f, z + 0.9f, 0.65f, 0.60f, 0.55f);
        }
    }

    // Research Lab D: Workbenches
    renderBox(6.0f, 0.0f, 6.0f, 15.0f, 0.85f, 7.5f, 0.30f, 0.40f, 0.45f);
    renderBox(6.0f, 0.0f, 13.0f, 15.0f, 0.85f, 14.5f, 0.30f, 0.40f, 0.45f);
}
