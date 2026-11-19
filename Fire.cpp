#include "Fire.h"
#include <cmath>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static float randFloat(float minVal, float maxVal) {
    return minVal + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxVal - minVal)));
}

FireHazard::FireHazard(int _id, const Vector3& pos, float r)
    : id(_id), position(pos), radius(r), isActive(false), animTimer(0.0f) {
    particles.resize(PARTICLE_COUNT);
    for (int i = 0; i < PARTICLE_COUNT; ++i) {
        initParticle(particles[i], true);
    }
}

void FireHazard::initParticle(FireParticle& p, bool randomAge) {
    // Spawn around center with radial distribution
    float angle = randFloat(0.0f, 2.0f * (float)M_PI);
    float dist = randFloat(0.0f, radius * 0.7f);

    p.x = position.x + cos(angle) * dist;
    p.y = position.y + randFloat(0.0f, 0.4f);
    p.z = position.z + sin(angle) * dist;

    // Upward velocity with slight turbulence
    p.vx = randFloat(-0.35f, 0.35f);
    p.vy = randFloat(1.2f, 2.6f);
    p.vz = randFloat(-0.35f, 0.35f);

    p.maxLife = randFloat(0.8f, 1.6f);
    p.life = randomAge ? randFloat(0.0f, 1.0f) : 0.0f;
    p.size = randFloat(0.18f, 0.38f);

    // Initial color: bright yellow-orange
    p.r = 1.0f;
    p.g = 0.9f;
    p.b = 0.2f;
    p.a = 0.85f;
}

void FireHazard::update(float deltaTime) {
    if (!isActive) return;

    animTimer += deltaTime;

    for (size_t i = 0; i < particles.size(); ++i) {
        FireParticle& p = particles[i];
        p.life += deltaTime / p.maxLife;

        if (p.life >= 1.0f) {
            initParticle(p, false);
            continue;
        }

        // Particle physics
        p.x += p.vx * deltaTime;
        p.y += p.vy * deltaTime;
        p.z += p.vz * deltaTime;

        // Color transition based on life
        // 0.0 - 0.3: Yellow to bright orange
        // 0.3 - 0.7: Orange to deep crimson red
        // 0.7 - 1.0: Dark smoke grey, fading opacity
        if (p.life < 0.35f) {
            float t = p.life / 0.35f;
            p.r = 1.0f;
            p.g = 0.9f - t * 0.4f; // 0.9 -> 0.5
            p.b = 0.2f - t * 0.2f; // 0.2 -> 0.0
            p.a = 0.85f;
            p.size += deltaTime * 0.15f;
        } else if (p.life < 0.75f) {
            float t = (p.life - 0.35f) / 0.4f;
            p.r = 1.0f - t * 0.25f; // 1.0 -> 0.75
            p.g = 0.5f - t * 0.45f; // 0.5 -> 0.05
            p.b = 0.02f;
            p.a = 0.85f - t * 0.35f;
            p.size += deltaTime * 0.2f;
        } else {
            // Smoke phase
            float t = (p.life - 0.75f) / 0.25f;
            p.r = 0.35f * (1.0f - t);
            p.g = 0.30f * (1.0f - t);
            p.b = 0.30f * (1.0f - t);
            p.a = 0.50f * (1.0f - t);
            p.size += deltaTime * 0.25f;
        }
    }
}

bool FireHazard::isPointInHazard(float px, float pz, float margin) const {
    if (!isActive) return false;
    float dx = px - position.x;
    float dz = pz - position.z;
    float effectiveRadius = radius + margin;
    return (dx * dx + dz * dz) <= (effectiveRadius * effectiveRadius);
}

void FireHazard::renderFloorDangerRing() {
    // Pulsing danger perimeter on the floor
    float pulse = 0.65f + 0.35f * sin(animTimer * 5.0f);

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. Semi-transparent red danger disc
    glColor4f(0.85f, 0.15f, 0.05f, 0.22f * pulse);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(position.x, position.y + 0.015f, position.z);
    int segments = 32;
    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * (float)M_PI * (float)i / (float)segments;
        glVertex3f(position.x + cos(theta) * radius, position.y + 0.015f, position.z + sin(theta) * radius);
    }
    glEnd();

    // 2. Bright pulsating perimeter ring
    glLineWidth(3.0f);
    glColor4f(1.0f, 0.25f, 0.1f, 0.85f * pulse);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; ++i) {
        float theta = 2.0f * (float)M_PI * (float)i / (float)segments;
        glVertex3f(position.x + cos(theta) * radius, position.y + 0.02f, position.z + sin(theta) * radius);
    }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void FireHazard::renderFlameCore() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for glowing fire

    // Dynamic flickering flame tongues (crossing vertical quads/triangles)
    int tongueCount = 6;
    for (int i = 0; i < tongueCount; ++i) {
        float baseAngle = (float)i * ((float)M_PI / (float)tongueCount) + animTimer * 0.8f;
        float h = 1.4f + 0.45f * sin(animTimer * 9.0f + (float)i * 1.5f);
        float w = radius * 0.65f * (0.85f + 0.15f * cos(animTimer * 7.0f + (float)i));

        float dx = cos(baseAngle) * w;
        float dz = sin(baseAngle) * w;

        glBegin(GL_TRIANGLES);
        // Bright white/yellow base
        glColor4f(1.0f, 0.95f, 0.4f, 0.9f);
        glVertex3f(position.x - dx, position.y + 0.05f, position.z - dz);
        glVertex3f(position.x + dx, position.y + 0.05f, position.z + dz);
        // Fiery red/orange tip
        glColor4f(1.0f, 0.2f, 0.0f, 0.0f);
        glVertex3f(position.x + sin(animTimer * 10.0f + (float)i) * 0.2f, position.y + h, position.z + cos(animTimer * 10.0f + (float)i) * 0.2f);
        glEnd();
    }

    // Inner bright core
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 1.0f, 0.8f, 0.95f);
    glVertex3f(position.x, position.y + 0.6f + 0.2f * sin(animTimer * 12.0f), position.z);
    glColor4f(1.0f, 0.4f, 0.05f, 0.2f);
    for (int i = 0; i <= 16; ++i) {
        float theta = 2.0f * (float)M_PI * (float)i / 16.0f;
        glVertex3f(position.x + cos(theta) * radius * 0.35f, position.y + 0.05f, position.z + sin(theta) * radius * 0.35f);
    }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void FireHazard::renderParticles() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Render individual glowing embers
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < particles.size(); ++i) {
        const FireParticle& p = particles[i];
        glColor4f(p.r, p.g, p.b, p.a);
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void FireHazard::render() {
    if (!isActive) return;

    renderFloorDangerRing();
    renderFlameCore();
    renderParticles();
}

// ==========================================
// FireManager Implementation
// ==========================================

FireManager::FireManager() : nextHazardIndex(0) {
}

FireManager::~FireManager() {
}

void FireManager::initialize() {
    setupPresetHazards();
}

void FireManager::setupPresetHazards() {
    hazards.clear();
    nextHazardIndex = 0;

    // Preset Hazard 1: Central intersection corridor (critical choke point)
    hazards.push_back(FireHazard(1, Vector3(0.0f, 0.0f, -5.0f), 2.2f));

    // Preset Hazard 2: East Cross Corridor (blocks direct route to East Exit)
    hazards.push_back(FireHazard(2, Vector3(7.0f, 0.0f, 0.0f), 2.3f));

    // Preset Hazard 3: Office A corridor doorway (forces NW room to use secondary door)
    hazards.push_back(FireHazard(3, Vector3(-2.5f, 0.0f, -10.5f), 2.2f));

    // Preset Hazard 4: South Central Corridor (blocks route to South Exit)
    hazards.push_back(FireHazard(4, Vector3(0.0f, 0.0f, 8.0f), 2.3f));

    // Preset Hazard 5: West Cross Corridor (blocks West Exit)
    hazards.push_back(FireHazard(5, Vector3(-9.0f, 0.0f, 0.0f), 2.3f));

    // Preset Hazard 6: Cafeteria entrance
    hazards.push_back(FireHazard(6, Vector3(-10.5f, 0.0f, 2.5f), 2.2f));

    // Initially activate Hazard 1 so there's an immediate fire threat to simulate
    if (!hazards.empty()) {
        hazards[0].isActive = true;
        nextHazardIndex = 1;
    }
}

void FireManager::reset() {
    for (size_t i = 0; i < hazards.size(); ++i) {
        hazards[i].isActive = false;
        hazards[i].animTimer = 0.0f;
    }
    // Re-activate first preset hazard
    if (!hazards.empty()) {
        hazards[0].isActive = true;
        nextHazardIndex = 1;
    } else {
        nextHazardIndex = 0;
    }
}

bool FireManager::activateNextHazard() {
    if (nextHazardIndex < hazards.size()) {
        hazards[nextHazardIndex].isActive = true;
        nextHazardIndex++;
        return true;
    }
    return false; // All preset hazards already active
}

void FireManager::addHazardAt(const Vector3& pos, float radius) {
    int newId = static_cast<int>(hazards.size() + 1);
    FireHazard h(newId, pos, radius);
    h.isActive = true;
    hazards.push_back(h);
}

void FireManager::update(float deltaTime) {
    for (size_t i = 0; i < hazards.size(); ++i) {
        if (hazards[i].isActive) {
            hazards[i].update(deltaTime);
        }
    }
}

void FireManager::render() {
    // Setup dynamic fire point light (GL_LIGHT1) at the first active fire
    for (size_t i = 0; i < hazards.size(); ++i) {
        if (hazards[i].isActive) {
            float flicker = 0.85f + 0.15f * sin(hazards[i].animTimer * 14.0f);
            GLfloat lightPos[] = { hazards[i].position.x, hazards[i].position.y + 1.2f, hazards[i].position.z, 1.0f };
            GLfloat lightAmbient[] = { 0.3f * flicker, 0.1f * flicker, 0.02f, 1.0f };
            GLfloat lightDiffuse[] = { 0.9f * flicker, 0.45f * flicker, 0.1f, 1.0f };
            GLfloat lightSpecular[] = { 0.8f * flicker, 0.3f * flicker, 0.05f, 1.0f };

            glEnable(GL_LIGHT1);
            glLightfv(GL_LIGHT1, GL_POSITION, lightPos);
            glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);
            glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
            glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
            glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
            glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.12f);
            glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.03f);
            break;
        }
    }

    // Render each active hazard
    for (size_t i = 0; i < hazards.size(); ++i) {
        if (hazards[i].isActive) {
            hazards[i].render();
        }
    }
}

bool FireManager::isPositionInFire(float x, float z, float margin) const {
    for (size_t i = 0; i < hazards.size(); ++i) {
        if (hazards[i].isActive && hazards[i].isPointInHazard(x, z, margin)) {
            return true;
        }
    }
    return false;
}

int FireManager::getActiveHazardCount() const {
    int count = 0;
    for (size_t i = 0; i < hazards.size(); ++i) {
        if (hazards[i].isActive) count++;
    }
    return count;
}
