#include "Person.h"
#include <cmath>
#include <cstdlib>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper to render colored box
static void drawBox(float w, float h, float d, float r, float g, float b) {
    GLfloat mat_ambient[] = { r * 0.35f, g * 0.35f, b * 0.35f, 1.0f };
    GLfloat mat_diffuse[] = { r, g, b, 1.0f };
    GLfloat mat_specular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);

    float hw = w * 0.5f;
    float hd = d * 0.5f;

    glBegin(GL_QUADS);
    // Front
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-hw, 0.0f, hd);
    glVertex3f(hw, 0.0f, hd);
    glVertex3f(hw, h, hd);
    glVertex3f(-hw, h, hd);

    // Back
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-hw, 0.0f, -hd);
    glVertex3f(-hw, h, -hd);
    glVertex3f(hw, h, -hd);
    glVertex3f(hw, 0.0f, -hd);

    // Top
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-hw, h, -hd);
    glVertex3f(-hw, h, hd);
    glVertex3f(hw, h, hd);
    glVertex3f(hw, h, -hd);

    // Bottom
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-hw, 0.0f, -hd);
    glVertex3f(hw, 0.0f, -hd);
    glVertex3f(hw, 0.0f, hd);
    glVertex3f(-hw, 0.0f, hd);

    // Right
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(hw, 0.0f, -hd);
    glVertex3f(hw, h, -hd);
    glVertex3f(hw, h, hd);
    glVertex3f(hw, 0.0f, hd);

    // Left
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-hw, 0.0f, -hd);
    glVertex3f(-hw, 0.0f, hd);
    glVertex3f(-hw, h, hd);
    glVertex3f(-hw, h, -hd);
    glEnd();
}

Person::Person(int _id, const Vector3& startPos, float r)
    : id(_id), position(startPos), velocity(0.0f, 0.0f, 0.0f),
      radius(r), facingAngle(0.0f), walkAnimPhase(0.0f), trappedTimer(0.0f),
      state(STATE_EVACUATING), targetExitId(-1), currentWaypointIdx(0),
      shirtR(0.18f), shirtG(0.48f), shirtB(0.85f),
      skinR(0.92f), skinG(0.76f), skinB(0.62f) {
    // Randomized slight speed variation (1.6 m/s to 2.2 m/s) for realistic crowd diversity
    speed = 1.7f + static_cast<float>(rand() % 100) / 100.0f * 0.5f;
    walkAnimPhase = static_cast<float>(rand() % 100);
}

Person::~Person() {
}

void Person::setShirtColor(float r, float g, float b) {
    shirtR = r;
    shirtG = g;
    shirtB = b;
}

bool Person::recalculateRoute(PathfindingGrid& grid, const std::vector<EmergencyExit>& exits) {
    if (state == STATE_EVACUATED) return true;

    int newExitId = -1;
    std::vector<Vector3> newPath;

    if (grid.findBestExitRoute(position, exits, newExitId, newPath)) {
        currentPath = newPath;
        targetExitId = newExitId;
        currentWaypointIdx = 0;
        state = STATE_EVACUATING;
        return true;
    } else {
        // No safe exit available - character is trapped!
        currentPath.clear();
        targetExitId = -1;
        state = STATE_TRAPPED;
        return false;
    }
}

void Person::applyCrowdSeparation(const std::vector<Person*>& allPeople, Vector3& steering) {
    float separationRadius = 0.65f;
    Vector3 repulsion(0.0f, 0.0f, 0.0f);
    int count = 0;

    for (size_t i = 0; i < allPeople.size(); ++i) {
        const Person* other = allPeople[i];
        if (other == this || other->state == STATE_EVACUATED) continue;

        float dx = position.x - other->position.x;
        float dz = position.z - other->position.z;
        float distSq = dx * dx + dz * dz;

        if (distSq > 0.0001f && distSq < separationRadius * separationRadius) {
            float dist = sqrt(distSq);
            float weight = (1.0f - dist / separationRadius);
            repulsion.x += (dx / dist) * weight * 1.5f;
            repulsion.z += (dz / dist) * weight * 1.5f;
            count++;
        }
    }

    if (count > 0) {
        steering.x += repulsion.x;
        steering.z += repulsion.z;
    }
}

void Person::update(float deltaTime, const Building& building, const FireManager& fireManager,
                    const std::vector<Person*>& allPeople) {
    if (state == STATE_EVACUATED) return;

    if (state == STATE_TRAPPED) {
        trappedTimer += deltaTime;
        velocity.x = 0.0f;
        velocity.z = 0.0f;
        return;
    }

    // 1. Waypoint Navigation
    if (currentWaypointIdx >= currentPath.size()) {
        // Person reached the final exit point
        state = STATE_EVACUATED;
        velocity.x = 0.0f;
        velocity.z = 0.0f;
        return;
    }

    Vector3 target = currentPath[currentWaypointIdx];
    float toTargetX = target.x - position.x;
    float toTargetZ = target.z - position.z;
    float distToTarget = sqrt(toTargetX * toTargetX + toTargetZ * toTargetZ);

    // If reached close to current waypoint, advance to next
    if (distToTarget < 0.35f) {
        currentWaypointIdx++;
        if (currentWaypointIdx >= currentPath.size()) {
            state = STATE_EVACUATED;
            velocity.x = 0.0f;
            velocity.z = 0.0f;
            return;
        }
        target = currentPath[currentWaypointIdx];
        toTargetX = target.x - position.x;
        toTargetZ = target.z - position.z;
        distToTarget = sqrt(toTargetX * toTargetX + toTargetZ * toTargetZ);
    }

    // Normalized direction to target
    Vector3 desiredDir(0.0f, 0.0f, 0.0f);
    if (distToTarget > 0.001f) {
        desiredDir.x = toTargetX / distToTarget;
        desiredDir.z = toTargetZ / distToTarget;
    }

    // Apply Crowd Separation
    applyCrowdSeparation(allPeople, desiredDir);

    // Normalize final movement direction
    float desiredLen = sqrt(desiredDir.x * desiredDir.x + desiredDir.z * desiredDir.z);
    if (desiredLen > 0.001f) {
        desiredDir.x /= desiredLen;
        desiredDir.z /= desiredLen;
    }

    // Update facing angle smoothly
    float targetAngle = atan2(desiredDir.x, desiredDir.z) * 180.0f / (float)M_PI;
    facingAngle = targetAngle;

    // Tentative next position
    float step = speed * deltaTime;
    float nextX = position.x + desiredDir.x * step;
    float nextZ = position.z + desiredDir.z * step;

    // Collision detection with walls
    bool collisionX = building.checkWallCollision(nextX, position.z, radius);
    bool collisionZ = building.checkWallCollision(position.x, nextZ, radius);

    if (!collisionX) {
        position.x = nextX;
    }
    if (!collisionZ) {
        position.z = nextZ;
    }

    // Prevent stepping into fire
    if (fireManager.isPositionInFire(position.x, position.z, radius)) {
        // Emergency pushback from fire center
        for (size_t i = 0; i < fireManager.getHazards().size(); ++i) {
            const FireHazard& h = fireManager.getHazards()[i];
            if (h.isActive && h.isPointInHazard(position.x, position.z, radius)) {
                float fx = position.x - h.position.x;
                float fz = position.z - h.position.z;
                float flen = sqrt(fx * fx + fz * fz);
                if (flen > 0.01f) {
                    position.x += (fx / flen) * step * 1.5f;
                    position.z += (fz / flen) * step * 1.5f;
                }
            }
        }
    }

    // Accumulate walking animation phase
    walkAnimPhase += deltaTime * speed * 6.5f;

    // Check if within arrival threshold of building perimeter exits
    if (fabs(position.x) >= 19.8f || fabs(position.z) >= 19.8f) {
        // If at an exit opening, mark safely evacuated
        if ((fabs(position.x) <= 2.0f && fabs(position.z) >= 19.8f) ||
            (fabs(position.z) <= 2.0f && fabs(position.x) >= 19.8f)) {
            state = STATE_EVACUATED;
        }
    }
}

void Person::renderCharacter3D() {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glRotatef(facingAngle, 0.0f, 1.0f, 0.0f);

    float legSwing = (state == STATE_EVACUATING) ? sin(walkAnimPhase) * 26.0f : 0.0f;
    float armSwing = (state == STATE_EVACUATING) ? -sin(walkAnimPhase) * 22.0f : 0.0f;

    // 1. Legs & Shoes (Dark trousers)
    float pantR = 0.15f, pantG = 0.18f, pantB = 0.25f;

    // Left Leg
    glPushMatrix();
    glTranslatef(-0.11f, 0.45f, 0.0f);
    glRotatef(legSwing, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -0.45f, 0.0f);
    drawBox(0.12f, 0.45f, 0.14f, pantR, pantG, pantB);
    // Left Shoe
    glTranslatef(0.0f, 0.0f, 0.03f);
    drawBox(0.12f, 0.08f, 0.18f, 0.1f, 0.1f, 0.1f);
    glPopMatrix();

    // Right Leg
    glPushMatrix();
    glTranslatef(0.11f, 0.45f, 0.0f);
    glRotatef(-legSwing, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -0.45f, 0.0f);
    drawBox(0.12f, 0.45f, 0.14f, pantR, pantG, pantB);
    // Right Shoe
    glTranslatef(0.0f, 0.0f, 0.03f);
    drawBox(0.12f, 0.08f, 0.18f, 0.1f, 0.1f, 0.1f);
    glPopMatrix();

    // 2. Torso (Blue Shirt / Emergency uniform)
    glPushMatrix();
    glTranslatef(0.0f, 0.46f, 0.0f);
    drawBox(0.34f, 0.55f, 0.20f, shirtR, shirtG, shirtB);
    glPopMatrix();

    // 3. Arms
    // Left Arm
    glPushMatrix();
    glTranslatef(-0.21f, 0.95f, 0.0f);
    glRotatef(armSwing, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -0.42f, 0.0f);
    drawBox(0.09f, 0.42f, 0.10f, shirtR * 0.9f, shirtG * 0.9f, shirtB * 0.9f);
    // Left Hand
    glTranslatef(0.0f, -0.06f, 0.0f);
    drawBox(0.08f, 0.06f, 0.08f, skinR, skinG, skinB);
    glPopMatrix();

    // Right Arm
    glPushMatrix();
    glTranslatef(0.21f, 0.95f, 0.0f);
    glRotatef(-armSwing, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -0.42f, 0.0f);
    drawBox(0.09f, 0.42f, 0.10f, shirtR * 0.9f, shirtG * 0.9f, shirtB * 0.9f);
    // Right Hand
    glTranslatef(0.0f, -0.06f, 0.0f);
    drawBox(0.08f, 0.06f, 0.08f, skinR, skinG, skinB);
    glPopMatrix();

    // 4. Head & Hair
    glPushMatrix();
    glTranslatef(0.0f, 1.05f, 0.0f);
    // Head cube
    drawBox(0.20f, 0.22f, 0.20f, skinR, skinG, skinB);
    // Hair on top
    glTranslatef(0.0f, 0.14f, -0.02f);
    drawBox(0.22f, 0.10f, 0.22f, 0.18f, 0.12f, 0.08f);
    glPopMatrix();

    glPopMatrix();
}

void Person::renderTrappedIndicator() {
    float pulse = 0.5f + 0.5f * sin(trappedTimer * 8.0f);

    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(position.x, position.y + 1.6f, position.z);

    // Floating red exclamation hazard marker
    glColor4f(1.0f * pulse, 0.1f, 0.1f, 0.9f);
    glLineWidth(3.0f);

    // Inverted Triangle
    glBegin(GL_LINE_LOOP);
    glVertex3f(-0.25f, 0.35f, 0.0f);
    glVertex3f(0.25f, 0.35f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glEnd();

    // Exclamation mark line & dot
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.28f, 0.0f);
    glVertex3f(0.0f, 0.12f, 0.0f);
    glEnd();
    glPointSize(4.0f);
    glBegin(GL_POINTS);
    glVertex3f(0.0f, 0.05f, 0.0f);
    glEnd();

    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void Person::render(bool showPath) {
    if (state == STATE_EVACUATED) return;

    // Render 3D Human character
    renderCharacter3D();

    // Render Trapped warning if blocked
    if (state == STATE_TRAPPED) {
        renderTrappedIndicator();
    }

    // Render evacuation route
    if (showPath && !currentPath.empty() && state == STATE_EVACUATING) {
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glColor4f(0.15f, 0.95f, 0.35f, 0.75f);
        glLineWidth(2.0f);

        glBegin(GL_LINE_STRIP);
        // Connect person feet to current and remaining waypoints
        glVertex3f(position.x, 0.06f, position.z);
        for (size_t i = currentWaypointIdx; i < currentPath.size(); ++i) {
            glVertex3f(currentPath[i].x, 0.06f, currentPath[i].z);
        }
        glEnd();

        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }
}
