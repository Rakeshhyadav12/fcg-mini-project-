#include <iostream>
#include <cstdlib>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "Simulation.h"

// Global Simulation Instance
static Simulation g_simulation;
static int g_windowWidth = 1280;
static int g_windowHeight = 720;
static int g_lastTimeMs = 0;

// Mouse Drag State
static bool g_mouseDragging = false;
static int g_lastMouseX = 0;
static int g_lastMouseY = 0;

void setupOpenGL() {
    // Enable Depth Testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Dark sleek visualization background
    glClearColor(0.04f, 0.05f, 0.08f, 1.0f);

    // Smooth shading & Face culling
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    // Enable Lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Sunlight / Overhead Ambient Directional Light (GL_LIGHT0)
    GLfloat light0_ambient[]  = { 0.28f, 0.30f, 0.36f, 1.0f };
    GLfloat light0_diffuse[]  = { 0.85f, 0.88f, 0.92f, 1.0f };
    GLfloat light0_specular[] = { 0.50f, 0.50f, 0.55f, 1.0f };
    GLfloat light0_position[] = { 15.0f, 35.0f, 20.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

    // Global Ambient Light
    GLfloat global_ambient[] = { 0.18f, 0.20f, 0.24f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    // Material tracking
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Anti-aliased lines and points
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render entire simulation (3D world + 2D HUD)
    g_simulation.render(g_windowWidth, g_windowHeight);

    glutSwapBuffers();
}

void reshape(int width, int height) {
    if (height <= 0) height = 1;
    g_windowWidth = width;
    g_windowHeight = height;
    glViewport(0, 0, width, height);
}

void timer(int value) {
    (void)value;
    int currentTimeMs = glutGet(GLUT_ELAPSED_TIME);
    if (g_lastTimeMs == 0) {
        g_lastTimeMs = currentTimeMs;
    }

    float deltaTime = static_cast<float>(currentTimeMs - g_lastTimeMs) / 1000.0f;
    g_lastTimeMs = currentTimeMs;

    // Cap delta time to prevent spiral of death during lag or window moving
    if (deltaTime > 0.1f) deltaTime = 0.1f;
    if (deltaTime < 0.0f) deltaTime = 0.016f;

    // Update simulation physics, AI, animations
    g_simulation.update(deltaTime);

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}

void keyboard(unsigned char key, int x, int y) {
    g_simulation.onKeyboard(key, x, y);
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    g_simulation.onSpecialKey(key, x, y);
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            g_mouseDragging = true;
            g_lastMouseX = x;
            g_lastMouseY = y;
        } else if (state == GLUT_UP) {
            g_mouseDragging = false;
        }
    } else if (button == 3) {
        // Scroll Wheel Up: Zoom in
        g_simulation.getCamera().zoom(-2.0f);
        glutPostRedisplay();
    } else if (button == 4) {
        // Scroll Wheel Down: Zoom out
        g_simulation.getCamera().zoom(2.0f);
        glutPostRedisplay();
    }
}

void motion(int x, int y) {
    if (g_mouseDragging) {
        int deltaX = x - g_lastMouseX;
        int deltaY = y - g_lastMouseY;

        g_simulation.getCamera().rotate(static_cast<float>(deltaX) * 0.4f,
                                        static_cast<float>(-deltaY) * 0.4f);

        g_lastMouseX = x;
        g_lastMouseY = y;
        glutPostRedisplay();
    }
}

void printBanner() {
    std::cout << "=================================================================\n";
    std::cout << "        3D AI-BASED EMERGENCY EVACUATION SIMULATOR              \n";
    std::cout << "          Developed using C++, OpenGL, and FreeGLUT             \n";
    std::cout << "=================================================================\n";
    std::cout << " FEATURES:\n";
    std::cout << "  * Full 3D Multi-Room Architectural Building with 4 Exits\n";
    std::cout << "  * Multi-Agent A* Shortest-Path Evacuation AI with Line-of-Sight\n";
    std::cout << "  * Dynamic Animated Fire Hazards & Particle Systems\n";
    std::cout << "  * Real-Time Hazard Rerouting & Trapped Citizen Detection\n";
    std::cout << "  * Continuous Wall Collision & Crowd Separation Dynamics\n";
    std::cout << "  * Dual Camera Modes: 3D Orbit Perspective & Top-Down Plan View\n";
    std::cout << "  * Comprehensive On-Screen Telemetry Dashboard & Alert System\n";
    std::cout << "-----------------------------------------------------------------\n";
    std::cout << " CONTROLS:\n";
    std::cout << "  [SPACE]      : Start / Pause Simulation\n";
    std::cout << "  [R]          : Restart Simulation\n";
    std::cout << "  [H]          : Activate / Spawn Additional Fire Hazard\n";
    std::cout << "  [C]          : Toggle Camera (3D Perspective <-> Top-Down View)\n";
    std::cout << "  [P]          : Toggle Evacuation Route Lines (Show/Hide)\n";
    std::cout << "  [ARROW KEYS] : Orbit Camera (Rotate Yaw / Pitch)\n";
    std::cout << "  [+ / =]      : Zoom In\n";
    std::cout << "  [- / _]      : Zoom Out\n";
    std::cout << "  [MOUSE DRAG] : Click & Drag Left Mouse Button to Orbit\n";
    std::cout << "  [ESC]        : Exit Application\n";
    std::cout << "=================================================================\n\n";
}

int main(int argc, char** argv) {
    printBanner();

    // 1. Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    // Center window on screen
    int screenW = glutGet(GLUT_SCREEN_WIDTH);
    int screenH = glutGet(GLUT_SCREEN_HEIGHT);
    int posX = (screenW > g_windowWidth) ? (screenW - g_windowWidth) / 2 : 50;
    int posY = (screenH > g_windowHeight) ? (screenH - g_windowHeight) / 2 : 50;

    glutInitWindowSize(g_windowWidth, g_windowHeight);
    glutInitWindowPosition(posX, posY);
    glutCreateWindow("3D AI-Based Emergency Evacuation Simulator");

    // 2. Setup OpenGL State
    setupOpenGL();

    // 3. Initialize Simulation
    g_simulation.initialize();

    // 4. Register GLUT Callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(16, timer, 0);

    // 5. Enter GLUT Main Event Loop
    glutMainLoop();

    return 0;
}
