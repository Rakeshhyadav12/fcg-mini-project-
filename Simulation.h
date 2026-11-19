#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <string>
#include "Building.h"
#include "Fire.h"
#include "Pathfinding.h"
#include "Person.h"
#include "Camera.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

class Simulation {
public:
    Simulation();
    ~Simulation();

    void initialize();
    void reset();
    void update(float deltaTime);
    void render(int windowWidth, int windowHeight);

    // User interactions
    void togglePause();
    void activateAdditionalHazard();
    void toggleCameraMode();
    void onKeyboard(unsigned char key, int x, int y);
    void onSpecialKey(int key, int x, int y);

    // Getters
    bool isPaused() const { return paused; }
    int getTotalPeople() const { return static_cast<int>(people.size()); }
    int getEvacuatedCount() const;
    int getTrappedCount() const;
    int getEvacuatingCount() const;
    float getSimulationTime() const { return simulationTime; }
    Camera& getCamera() { return camera; }

private:
    Building building;
    FireManager fireManager;
    PathfindingGrid grid;
    std::vector<Person*> people;
    Camera camera;

    bool paused;
    float simulationTime;
    float alertPulseTimer;
    bool showPathLines;

    // Internal initialization
    void spawnPeople();
    void recalculateAllRoutes();

    // 2D Dashboard rendering
    void renderDashboard(int screenWidth, int screenHeight);
    void drawPanel2D(float x, float y, float w, float h, float r, float g, float b, float a);
    void drawBorder2D(float x, float y, float w, float h, float r, float g, float b, float a, float lineWidth = 1.5f);
    void renderText2D(float x, float y, const std::string& text, void* font, float r, float g, float b);
};

#endif // SIMULATION_H
