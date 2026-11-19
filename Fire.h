#ifndef FIRE_H
#define FIRE_H

#include <vector>
#include "Building.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// Particle for realistic fire effect
struct FireParticle {
    float x, y, z;
    float vx, vy, vz;
    float life;      // 0.0 (born) to 1.0 (dead)
    float maxLife;   // seconds
    float size;
    float r, g, b, a;
};

// Represents a single active or pending fire hazard
class FireHazard {
public:
    int id;
    Vector3 position;
    float radius;
    bool isActive;
    float animTimer;

    FireHazard(int _id, const Vector3& pos, float r = 2.2f);

    void update(float deltaTime);
    void render();
    bool isPointInHazard(float px, float pz, float margin = 0.0f) const;

private:
    std::vector<FireParticle> particles;
    static const int PARTICLE_COUNT = 90;

    void initParticle(FireParticle& p, bool randomAge = false);
    void renderFloorDangerRing();
    void renderFlameCore();
    void renderParticles();
};

// Manages all fire hazards in the simulation
class FireManager {
public:
    FireManager();
    ~FireManager();

    void initialize();
    void update(float deltaTime);
    void render();
    void reset();

    // Spawn / Activate additional hazard
    bool activateNextHazard();
    void addHazardAt(const Vector3& pos, float radius = 2.2f);

    // Hazard query methods
    bool isPositionInFire(float x, float z, float margin = 0.5f) const;
    int getActiveHazardCount() const;
    const std::vector<FireHazard>& getHazards() const { return hazards; }
    std::vector<FireHazard>& getHazards() { return hazards; }

private:
    std::vector<FireHazard> hazards;
    size_t nextHazardIndex;
    void setupPresetHazards();
};

#endif // FIRE_H
