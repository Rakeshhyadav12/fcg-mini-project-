#ifndef CAMERA_H
#define CAMERA_H

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

enum CameraMode {
    CAMERA_3D_PERSPECTIVE,
    CAMERA_TOP_DOWN
};

class Camera {
public:
    Camera();
    ~Camera();

    void reset();

    // Manipulation
    void rotate(float deltaYaw, float deltaPitch);
    void zoom(float deltaDistance);
    void moveTarget(float deltaX, float deltaZ);
    void toggleMode();
    void setMode(CameraMode newMode);

    // OpenGL Matrices
    void applyProjection(int screenWidth, int screenHeight);
    void applyView();

    // Getters
    CameraMode getMode() const { return mode; }
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }
    float getDistance() const { return distance; }

private:
    CameraMode mode;

    // Orbit parameters
    float targetX, targetY, targetZ;
    float distance;
    float yaw;     // Horizontal angle in degrees
    float pitch;   // Vertical angle in degrees

    // Top-down parameters
    float topDownHeight;

    void clampParameters();
};

#endif // CAMERA_H
