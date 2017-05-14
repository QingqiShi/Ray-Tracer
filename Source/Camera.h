#ifndef __H_CAMERA_H__
#define __H_CAMERA_H__

#include <glm/gtx/rotate_vector.hpp>

class Camera {
public:
    float focalLength;
    vec3 position;

    float speed;
    float rSpeed;

    float yaw;
    float pitch;
    // mat3 rotationY;
    // mat3 rotationX;

    Camera(float x, float y, float z, float focalLength)
        : focalLength(focalLength), yaw(0), pitch(0) {
        position.x = x;
        position.y = y;
        position.z = z;
        speed = 1;
        rSpeed = 0.1;
        // rotationY = mat3(1.0);
        // rotationX = mat3(1.0);
    }

    void Translate(vec3 u) {
        // position += u * this->rotationY * this->rotationX;
        position += CameraToWorld(u, false);
    }

    void Rotate(float deltaYaw) {
        yaw += deltaYaw;

        // rotationY[0][0] = cos(yaw);
        // rotationY[2][0] = sin(yaw);
        // rotationY[0][2] = -sin(yaw);
        // rotationY[2][2] = cos(yaw);
    }

    void Rotate(float deltaYaw, float deltaPitch) {
        Rotate(deltaYaw);

        pitch += deltaPitch;

        // rotationX[1][1] = cos(pitch);
        // rotationX[2][1] = -sin(pitch);
        // rotationX[1][2] = sin(pitch);
        // rotationX[2][2] = cos(pitch);
    }

    vec3 WorldToCamera(vec3 pointInWorldSpace) {
        // return ((pointInWorldSpace - this->position) * this->rotationY) * this->rotationX;
        return rotateX(rotateY(pointInWorldSpace - position, yaw), pitch);
    }

    vec3 WorldToCamera(vec3 pointInWorldSpace, bool applyTranslate) {
        if (applyTranslate) {
            return WorldToCamera(pointInWorldSpace);
        } else {
            // return (pointInWorldSpace * this->rotationY) * this->rotationX;
            return rotateX(rotateY(pointInWorldSpace, yaw), pitch);
        }
    }

    vec3 CameraToWorld(vec3 pointInCameraSpace, bool applyTranslation) {
        if (applyTranslation) {
            return rotateY(rotateX(pointInCameraSpace, -pitch), -yaw) + position;
        } else {
            return rotateY(rotateX(pointInCameraSpace, -pitch), -yaw);
        }
    }

    vec2 WorldToScreen(vec3 pointInWorldSpace) {
        int x, y;

        vec3 p = WorldToCamera(pointInWorldSpace);

        x = this->focalLength * p.x / abs(p.z) + SCREEN_WIDTH / 2.f;
        y = this->focalLength * p.y / abs(p.z) + SCREEN_HEIGHT / 2.f;

        return ivec2(x, y);
    }
};

#endif
