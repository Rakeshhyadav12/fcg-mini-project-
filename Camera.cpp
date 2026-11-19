#include "Camera.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Camera::Camera() {
    reset();
}

Camera::~Camera() {
}

void Camera::reset() {
    mode = CAMERA_3D_PERSPECTIVE;
    targetX = 0.0f;
    targetY = 0.0f;
    targetZ = 0.0f;
    distance = 38.0f;
    yaw = 45.0f;
    pitch = 38.0f;
    topDownHeight = 44.0f;
}

void Camera::clampParameters() {
    if (pitch < 12.0f) pitch = 12.0f;
    if (pitch > 85.0f) pitch = 85.0f;

    if (distance < 12.0f) distance = 12.0f;
    if (distance > 70.0f) distance = 70.0f;

    if (topDownHeight < 15.0f) topDownHeight = 15.0f;
    if (topDownHeight > 80.0f) topDownHeight = 80.0f;

    while (yaw >= 360.0f) yaw -= 360.0f;
    while (yaw < 0.0f) yaw += 360.0f;
}

void Camera::rotate(float deltaYaw, float deltaPitch) {
    if (mode == CAMERA_3D_PERSPECTIVE) {
        yaw += deltaYaw;
        pitch += deltaPitch;
        clampParameters();
    }
}

void Camera::zoom(float deltaDistance) {
    if (mode == CAMERA_3D_PERSPECTIVE) {
        distance += deltaDistance;
    } else {
        topDownHeight += deltaDistance * 1.2f;
    }
    clampParameters();
}

void Camera::moveTarget(float deltaX, float deltaZ) {
    // Transform delta along current camera orientation
    float yawRad = yaw * static_cast<float>(M_PI) / 180.0f;
    float forwardX = sin(yawRad);
    float forwardZ = cos(yawRad);
    float rightX = cos(yawRad);
    float rightZ = -sin(yawRad);

    targetX += rightX * deltaX + forwardX * deltaZ;
    targetZ += rightZ * deltaX + forwardZ * deltaZ;

    // Keep target reasonably centered around building
    if (targetX < -15.0f) targetX = -15.0f;
    if (targetX > 15.0f) targetX = 15.0f;
    if (targetZ < -15.0f) targetZ = -15.0f;
    if (targetZ > 15.0f) targetZ = 15.0f;
}

void Camera::toggleMode() {
    if (mode == CAMERA_3D_PERSPECTIVE) {
        mode = CAMERA_TOP_DOWN;
    } else {
        mode = CAMERA_3D_PERSPECTIVE;
    }
}

void Camera::setMode(CameraMode newMode) {
    mode = newMode;
}

void Camera::applyProjection(int screenWidth, int screenHeight) {
    if (screenHeight <= 0) screenHeight = 1;
    float aspect = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (mode == CAMERA_3D_PERSPECTIVE) {
        gluPerspective(45.0, aspect, 0.5, 350.0);
    } else {
        // High-altitude top-down perspective view
        gluPerspective(48.0, aspect, 0.5, 350.0);
    }

    glMatrixMode(GL_MODELVIEW);
}

void Camera::applyView() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (mode == CAMERA_3D_PERSPECTIVE) {
        float yawRad = yaw * static_cast<float>(M_PI) / 180.0f;
        float pitchRad = pitch * static_cast<float>(M_PI) / 180.0f;

        float eyeX = targetX + distance * cos(pitchRad) * sin(yawRad);
        float eyeY = targetY + distance * sin(pitchRad);
        float eyeZ = targetZ + distance * cos(pitchRad) * cos(yawRad);

        gluLookAt(eyeX, eyeY, eyeZ,
                  targetX, targetY, targetZ,
                  0.0f, 1.0f, 0.0f);
    } else {
        // Direct overhead plan view looking down along negative Y
        gluLookAt(targetX, targetY + topDownHeight, targetZ,
                  targetX, targetY, targetZ,
                  0.0f, 0.0f, -1.0f);
    }
}
