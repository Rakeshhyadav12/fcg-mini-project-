#ifndef PERSON_H
#define PERSON_H

#include <vector>
#include "Building.h"
#include "Pathfinding.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

enum PersonState {
    STATE_EVACUATING,
    STATE_EVACUATED,
    STATE_TRAPPED
};

class Person {
public:
    int id;
    Vector3 position;
    Vector3 velocity;
    float radius;
    float speed;
    float facingAngle;       // Degrees in Y-axis
    float walkAnimPhase;     // Animation cycle for walking limbs
    float trappedTimer;      // Timer for panic blinking

    PersonState state;
    int targetExitId;
    std::vector<Vector3> currentPath;
    size_t currentWaypointIdx;

    // Visual attributes
    float shirtR, shirtG, shirtB;
    float skinR, skinG, skinB;

    Person(int _id, const Vector3& startPos, float r = 0.35f);
    ~Person();

    void update(float deltaTime, const Building& building, const FireManager& fireManager,
                const std::vector<Person*>& allPeople);
    void render(bool showPath = true);

    bool recalculateRoute(PathfindingGrid& grid, const std::vector<EmergencyExit>& exits);
    void setShirtColor(float r, float g, float b);

    bool isEvacuated() const { return state == STATE_EVACUATED; }
    bool isTrapped() const { return state == STATE_TRAPPED; }
    bool isEvacuating() const { return state == STATE_EVACUATING; }

private:
    void renderCharacter3D();
    void renderTrappedIndicator();
    void applyCrowdSeparation(const std::vector<Person*>& allPeople, Vector3& steering);
};

#endif // PERSON_H
