#include "Simulation.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <iostream>

Simulation::Simulation()
    : paused(false), simulationTime(0.0f), alertPulseTimer(0.0f), showPathLines(true) {
}

Simulation::~Simulation() {
    for (size_t i = 0; i < people.size(); ++i) {
        delete people[i];
    }
    people.clear();
}

void Simulation::initialize() {
    // 1. Initialize building floor plan
    building.initialize();

    // 2. Initialize fire hazard system
    fireManager.initialize();

    // 3. Initialize navigation pathfinding grid from building walls
    grid.initialize(building);
    grid.updateHazards(fireManager);

    // 4. Initialize camera
    camera.reset();

    // 5. Spawn people and calculate initial evacuation routes
    spawnPeople();

    paused = false;
    simulationTime = 0.0f;
    alertPulseTimer = 0.0f;
    showPathLines = true;
}

void Simulation::spawnPeople() {
    // Clean up any existing people
    for (size_t i = 0; i < people.size(); ++i) {
        delete people[i];
    }
    people.clear();

    // Diverse blue uniform palettes
    float shirtColors[][3] = {
        { 0.12f, 0.45f, 0.85f }, // Royal Blue
        { 0.05f, 0.65f, 0.85f }, // Cyan
        { 0.18f, 0.32f, 0.75f }, // Cobalt
        { 0.20f, 0.55f, 0.70f }, // Teal Blue
        { 0.10f, 0.25f, 0.60f }, // Navy Blue
        { 0.35f, 0.60f, 0.90f }, // Sky Blue
        { 0.25f, 0.45f, 0.78f }, // Slate Blue
        { 0.08f, 0.50f, 0.75f }  // Ocean Blue
    };
    int numColors = sizeof(shirtColors) / sizeof(shirtColors[0]);

    // Strategic spawn points across all 4 rooms and corridors
    Vector3 spawnPositions[] = {
        // Office A (NW Room)
        Vector3(-14.0f, 0.0f, -14.0f),
        Vector3(-10.0f, 0.0f, -16.0f),
        Vector3(-8.0f,  0.0f, -12.0f),
        Vector3(-16.0f, 0.0f, -8.0f),

        // Server Lab B (NE Room)
        Vector3(12.0f,  0.0f, -14.0f),
        Vector3(8.0f,   0.0f, -16.0f),
        Vector3(15.0f,  0.0f, -10.0f),
        Vector3(9.0f,   0.0f, -7.0f),

        // Cafeteria C (SW Room)
        Vector3(-14.0f, 0.0f, 14.0f),
        Vector3(-8.0f,  0.0f, 15.0f),
        Vector3(-15.0f, 0.0f, 8.0f),
        Vector3(-10.0f, 0.0f, 9.0f),

        // Research Lab D (SE Room)
        Vector3(12.0f,  0.0f, 14.0f),
        Vector3(8.0f,   0.0f, 16.0f),
        Vector3(14.0f,  0.0f, 9.0f),
        Vector3(10.0f,  0.0f, 11.0f)
    };
    int totalSpawns = sizeof(spawnPositions) / sizeof(spawnPositions[0]);

    for (int i = 0; i < totalSpawns; ++i) {
        Person* p = new Person(i + 1, spawnPositions[i]);
        int cIdx = i % numColors;
        p->setShirtColor(shirtColors[cIdx][0], shirtColors[cIdx][1], shirtColors[cIdx][2]);
        people.push_back(p);
    }

    // Compute initial shortest safe evacuation routes
    recalculateAllRoutes();
}

void Simulation::recalculateAllRoutes() {
    // 1. Update navigation grid with latest hazards
    grid.updateHazards(fireManager);

    // 2. Check if any exits are directly blocked by fire
    std::vector<EmergencyExit>& exits = building.getExits();
    for (size_t i = 0; i < exits.size(); ++i) {
        exits[i].isBlocked = fireManager.isPositionInFire(exits[i].position.x, exits[i].position.z, 1.2f);
    }

    // 3. Recalculate routes for all active people
    for (size_t i = 0; i < people.size(); ++i) {
        if (!people[i]->isEvacuated()) {
            people[i]->recalculateRoute(grid, exits);
        }
    }
}

void Simulation::reset() {
    fireManager.reset();
    grid.initialize(building);
    grid.updateHazards(fireManager);
    spawnPeople();
    simulationTime = 0.0f;
    alertPulseTimer = 0.0f;
    paused = false;
}

void Simulation::togglePause() {
    paused = !paused;
}

void Simulation::activateAdditionalHazard() {
    bool activated = fireManager.activateNextHazard();
    if (activated) {
        recalculateAllRoutes();
    }
}

void Simulation::toggleCameraMode() {
    camera.toggleMode();
}

void Simulation::onKeyboard(unsigned char key, int x, int y) {
    (void)x; (void)y;
    switch (key) {
        case ' ':
            togglePause();
            break;
        case 'r':
        case 'R':
            reset();
            break;
        case 'h':
        case 'H':
            activateAdditionalHazard();
            break;
        case 'c':
        case 'C':
            toggleCameraMode();
            break;
        case 'p':
        case 'P':
            showPathLines = !showPathLines;
            break;
        case '+':
        case '=':
            camera.zoom(-2.5f);
            break;
        case '-':
        case '_':
            camera.zoom(2.5f);
            break;
        case 27: // ESC key
            exit(0);
            break;
        default:
            break;
    }
}

void Simulation::onSpecialKey(int key, int x, int y) {
    (void)x; (void)y;
    float rotateSpeed = 4.0f;
    switch (key) {
        case GLUT_KEY_LEFT:
            camera.rotate(-rotateSpeed, 0.0f);
            break;
        case GLUT_KEY_RIGHT:
            camera.rotate(rotateSpeed, 0.0f);
            break;
        case GLUT_KEY_UP:
            camera.rotate(0.0f, rotateSpeed);
            break;
        case GLUT_KEY_DOWN:
            camera.rotate(0.0f, -rotateSpeed);
            break;
        case GLUT_KEY_PAGE_UP:
            camera.zoom(-2.5f);
            break;
        case GLUT_KEY_PAGE_DOWN:
            camera.zoom(2.5f);
            break;
        default:
            break;
    }
}

void Simulation::update(float deltaTime) {
    alertPulseTimer += deltaTime;

    // Building update (animated exit signs)
    building.update(deltaTime);

    // Fire animation update
    fireManager.update(deltaTime);

    if (paused) return;

    simulationTime += deltaTime;

    // Update people movement and collisions
    for (size_t i = 0; i < people.size(); ++i) {
        people[i]->update(deltaTime, building, fireManager, people);
    }
}

int Simulation::getEvacuatedCount() const {
    int count = 0;
    for (size_t i = 0; i < people.size(); ++i) {
        if (people[i]->isEvacuated()) count++;
    }
    return count;
}

int Simulation::getTrappedCount() const {
    int count = 0;
    for (size_t i = 0; i < people.size(); ++i) {
        if (people[i]->isTrapped()) count++;
    }
    return count;
}

int Simulation::getEvacuatingCount() const {
    int count = 0;
    for (size_t i = 0; i < people.size(); ++i) {
        if (people[i]->isEvacuating()) count++;
    }
    return count;
}

void Simulation::render(int windowWidth, int windowHeight) {
    // 1. Setup 3D Scene Camera
    camera.applyProjection(windowWidth, windowHeight);
    camera.applyView();

    // 2. Render Building Environment
    building.render();

    // 3. Render Active Fire Hazards
    fireManager.render();

    // 4. Render Human Characters and their Evacuation Routes
    for (size_t i = 0; i < people.size(); ++i) {
        people[i]->render(showPathLines);
    }

    // 5. Render 2D Dashboard & HUD
    renderDashboard(windowWidth, windowHeight);
}

// ==========================================
// 2D Dashboard and HUD Overlay Implementation
// ==========================================

void Simulation::drawPanel2D(float x, float y, float w, float h, float r, float g, float b, float a) {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();

    glDisable(GL_BLEND);
}

void Simulation::drawBorder2D(float x, float y, float w, float h, float r, float g, float b, float a, float lineWidth) {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glLineWidth(lineWidth);
    glColor4f(r, g, b, a);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();

    glDisable(GL_BLEND);
}

void Simulation::renderText2D(float x, float y, const std::string& text, void* font, float r, float g, float b) {
    glDisable(GL_LIGHTING);
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (size_t i = 0; i < text.length(); ++i) {
        glutBitmapCharacter(font, text[i]);
    }
}

void Simulation::renderDashboard(int screenWidth, int screenHeight) {
    // Switch to Orthographic 2D Projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, static_cast<double>(screenWidth), static_cast<double>(screenHeight), 0.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    // 1. Top Title Bar
    drawPanel2D(0.0f, 0.0f, static_cast<float>(screenWidth), 42.0f, 0.04f, 0.06f, 0.10f, 0.90f);
    drawBorder2D(0.0f, 0.0f, static_cast<float>(screenWidth), 42.0f, 0.12f, 0.35f, 0.65f, 0.85f, 1.5f);

    renderText2D(18.0f, 26.0f, "3D AI-BASED EMERGENCY EVACUATION SIMULATOR",
                 GLUT_BITMAP_HELVETICA_18, 0.2f, 0.85f, 1.0f);

    // Operational Status pill on top right
    float statusX = static_cast<float>(screenWidth) - 220.0f;
    if (paused) {
        drawPanel2D(statusX, 8.0f, 200.0f, 26.0f, 0.45f, 0.35f, 0.05f, 0.85f);
        drawBorder2D(statusX, 8.0f, 200.0f, 26.0f, 1.0f, 0.85f, 0.1f, 1.0f, 1.5f);
        renderText2D(statusX + 28.0f, 25.0f, "STATUS: PAUSED", GLUT_BITMAP_HELVETICA_12, 1.0f, 0.95f, 0.2f);
    } else {
        drawPanel2D(statusX, 8.0f, 200.0f, 26.0f, 0.05f, 0.35f, 0.15f, 0.85f);
        drawBorder2D(statusX, 8.0f, 200.0f, 26.0f, 0.2f, 0.95f, 0.35f, 1.0f, 1.5f);
        renderText2D(statusX + 24.0f, 25.0f, "STATUS: SIMULATING", GLUT_BITMAP_HELVETICA_12, 0.2f, 1.0f, 0.4f);
    }

    // 2. Left Telemetry Dashboard Panel
    float panelX = 18.0f;
    float panelY = 56.0f;
    float panelW = 320.0f;
    float panelH = 310.0f;

    drawPanel2D(panelX, panelY, panelW, panelH, 0.03f, 0.05f, 0.09f, 0.88f);
    drawBorder2D(panelX, panelY, panelW, panelH, 0.15f, 0.45f, 0.70f, 0.75f, 1.5f);

    // Dashboard Header
    renderText2D(panelX + 16.0f, panelY + 28.0f, "EVACUATION TELEMETRY", GLUT_BITMAP_HELVETICA_12, 0.15f, 0.85f, 1.0f);
    drawBorder2D(panelX + 16.0f, panelY + 36.0f, panelW - 32.0f, 0.0f, 0.2f, 0.4f, 0.6f, 0.6f, 1.0f);

    // Telemetry items
    char buffer[128];
    float lineY = panelY + 62.0f;
    float lineStep = 24.0f;

    // Simulation Timer
    int minutes = static_cast<int>(simulationTime) / 60;
    float seconds = fmod(simulationTime, 60.0f);
    snprintf(buffer, sizeof(buffer), "Simulation Time:    %02d:%04.1f s", minutes, seconds);
    renderText2D(panelX + 16.0f, lineY, buffer, GLUT_BITMAP_9_BY_15, 0.9f, 0.9f, 0.95f);
    lineY += lineStep;

    // Total People
    snprintf(buffer, sizeof(buffer), "Total Evacuees:     %d", getTotalPeople());
    renderText2D(panelX + 16.0f, lineY, buffer, GLUT_BITMAP_9_BY_15, 0.85f, 0.85f, 0.85f);
    lineY += lineStep;

    // Evacuated
    int evacCount = getEvacuatedCount();
    float pct = (getTotalPeople() > 0) ? (static_cast<float>(evacCount) / static_cast<float>(getTotalPeople()) * 100.0f) : 0.0f;
    snprintf(buffer, sizeof(buffer), "Evacuated Safely:   %d (%.1f%%)", evacCount, pct);
    renderText2D(panelX + 16.0f, lineY, buffer, GLUT_BITMAP_9_BY_15, 0.2f, 1.0f, 0.35f);
    lineY += lineStep;

    // Currently Evacuating
    int inTransit = getEvacuatingCount();
    snprintf(buffer, sizeof(buffer), "Currently Moving:   %d", inTransit);
    renderText2D(panelX + 16.0f, lineY, buffer, GLUT_BITMAP_9_BY_15, 0.25f, 0.75f, 1.0f);
    lineY += lineStep;

    // Trapped People
    int trapped = getTrappedCount();
    snprintf(buffer, sizeof(buffer), "Trapped / Blocked:  %d", trapped);
    if (trapped > 0) {
        float pulse = 0.5f + 0.5f * sin(alertPulseTimer * 8.0f);
        renderText2D(panelX + 16.0f, lineY, buffer, GLUT_BITMAP_9_BY_15, 1.0f, 0.2f * pulse, 0.2f * pulse);
    } else {
        renderText2D(panelX + 16.0f, lineY, buffer, GLUT_BITMAP_9_BY_15, 0.7f, 0.7f, 0.75f);
    }
    lineY += lineStep;

    // Active Hazards
    int hazardsCount = fireManager.getActiveHazardCount();
    snprintf(buffer, sizeof(buffer), "Active Fire Hazards: %d", hazardsCount);
    renderText2D(panelX + 16.0f, lineY, buffer, GLUT_BITMAP_9_BY_15, 1.0f, 0.55f, 0.15f);
    lineY += lineStep;

    // Safe Exits
    int safeExits = 0;
    for (size_t i = 0; i < building.getExits().size(); ++i) {
        if (!building.getExits()[i].isBlocked) safeExits++;
    }
    snprintf(buffer, sizeof(buffer), "Operational Exits:  %d / %d", safeExits, static_cast<int>(building.getExits().size()));
    renderText2D(panelX + 16.0f, lineY, buffer, GLUT_BITMAP_9_BY_15, (safeExits > 0) ? 0.2f : 1.0f, (safeExits > 0) ? 1.0f : 0.2f, 0.2f);
    lineY += lineStep;

    // Camera Mode
    std::string camStr = (camera.getMode() == CAMERA_3D_PERSPECTIVE) ? "3D Perspective" : "Top-Down View";
    snprintf(buffer, sizeof(buffer), "Camera View:        %s", camStr.c_str());
    renderText2D(panelX + 16.0f, lineY, buffer, GLUT_BITMAP_9_BY_15, 0.75f, 0.85f, 0.95f);
    lineY += lineStep;

    // Path Lines Status
    snprintf(buffer, sizeof(buffer), "Evacuation Routes:  %s", showPathLines ? "VISIBLE" : "HIDDEN");
    renderText2D(panelX + 16.0f, lineY, buffer, GLUT_BITMAP_9_BY_15, showPathLines ? 0.3f : 0.6f, showPathLines ? 0.9f : 0.6f, 0.4f);

    // 3. Dynamic Warning Banner (When someone is trapped)
    if (trapped > 0) {
        float pulse = 0.5f + 0.5f * sin(alertPulseTimer * 8.0f);
        float bannerW = 620.0f;
        float bannerH = 40.0f;
        float bannerX = (static_cast<float>(screenWidth) - bannerW) * 0.5f;
        float bannerY = 54.0f;

        drawPanel2D(bannerX, bannerY, bannerW, bannerH, 0.5f * pulse, 0.05f, 0.05f, 0.92f);
        drawBorder2D(bannerX, bannerY, bannerW, bannerH, 1.0f, 0.2f * pulse, 0.2f * pulse, 1.0f, 2.0f);

        snprintf(buffer, sizeof(buffer), "[ ! ] CRITICAL ALERT: %d EVACUEE(S) TRAPPED - ALL ROUTES BLOCKED BY HAZARD!", trapped);
        renderText2D(bannerX + 24.0f, bannerY + 25.0f, buffer, GLUT_BITMAP_HELVETICA_12, 1.0f, 0.9f, 0.9f);
    } else if (evacCount == getTotalPeople() && getTotalPeople() > 0) {
        // Evacuation Complete Banner
        float bannerW = 560.0f;
        float bannerH = 40.0f;
        float bannerX = (static_cast<float>(screenWidth) - bannerW) * 0.5f;
        float bannerY = 54.0f;

        drawPanel2D(bannerX, bannerY, bannerW, bannerH, 0.05f, 0.45f, 0.15f, 0.90f);
        drawBorder2D(bannerX, bannerY, bannerW, bannerH, 0.2f, 1.0f, 0.4f, 1.0f, 2.0f);

        renderText2D(bannerX + 32.0f, bannerY + 25.0f, "[ * ] EVACUATION OPERATION COMPLETE: ALL CIVILIANS SAFELY ESCAPED!",
                     GLUT_BITMAP_HELVETICA_12, 0.9f, 1.0f, 0.9f);
    }

    // 4. Bottom User Interaction Control Bar
    float bottomH = 34.0f;
    float bottomY = static_cast<float>(screenHeight) - bottomH;
    drawPanel2D(0.0f, bottomY, static_cast<float>(screenWidth), bottomH, 0.03f, 0.04f, 0.08f, 0.92f);
    drawBorder2D(0.0f, bottomY, static_cast<float>(screenWidth), bottomH, 0.15f, 0.35f, 0.55f, 0.80f, 1.0f);

    std::string controlGuide = "[SPACE] Pause/Resume   [R] Restart   [H] Add Hazard   [C] Camera View   [ARROWS] Rotate   [+/-] Zoom   [P] Paths   [ESC] Exit";
    renderText2D(20.0f, bottomY + 22.0f, controlGuide, GLUT_BITMAP_HELVETICA_12, 0.75f, 0.85f, 0.95f);

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
