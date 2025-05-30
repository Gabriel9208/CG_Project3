#include "Camera.h"
#include <glm/glm.hpp>

namespace CG
{
    void Camera::updateVectors(const glm::quat deltaOrientation)
    {
        glm::vec3 direction = pos - target;
        glm::vec3 rotatedDirection = deltaOrientation * direction;

        pos = rotatedDirection + target;
        up = deltaOrientation * up;
        LookAt();

    }
    Camera::Camera()
    {
        projection = glm::mat4(1.0);
        view = glm::mat4(1.0);

        fov = 80.0f;
        aspect = 4.0f / 3.0f;
        clipNear = 0.01f;
        clipFar = 1000.0f;

        UpdateProjectionMatrix();
    }

    Camera::~Camera()
    {

    }

    void Camera::LookAt()
    {
        view = glm::lookAt(pos, target, up);
    }

    void Camera::LookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 _up)
    {
        pos = eye;
        target = center;
        up = _up;

        LookAt();
    }

    void Camera::rotateAround(float angle, glm::vec3 axis)
    {
        if (axis == glm::vec3(1, 0, 0)) {
            axis = glm::normalize(glm::cross((target - pos), up));

        }
        glm::quat deltaOrientation = glm::angleAxis(glm::radians(angle), axis);

        updateVectors(deltaOrientation);
    }


    void Camera::setPos(glm::vec3 newPos)
    {
        pos = newPos;
        LookAt();
    }

    void Camera::setTarget(glm::vec3 newTarget)
    {
        target = newTarget;
        LookAt();
    }

    void Camera::setUpVector(glm::vec3 newUp)
    {
        up = newUp;
        LookAt();
    }

    void Camera::flatTranslate(float wRight, float wUp)
    {
        glm::vec3 direction = pos - target;
        glm::vec3 right = glm::normalize(glm::cross(up, direction));

        setPos(glm::vec3(pos[0] + right[0] * wRight + up[0] * wUp,
            pos[1] + right[1] * wRight + up[1] * wUp,
            pos[2] + right[2] * wRight + up[2] * wUp));

        setTarget(glm::vec3(target[0] + right[0] * wRight + up[0] * wUp,
            target[1] + right[1] * wRight + up[1] * wUp,
            target[2] + right[2] * wRight + up[2] * wUp));
    }

    void Camera::zoom(float weight)
    {
        glm::vec3 direction = pos - target;
        direction = glm::normalize(direction);
        setPos(glm::vec3(pos[0] + direction[0] * weight,
            pos[1] + direction[1] * weight,
            pos[2] + direction[2] * weight));
    }

    void Camera::UpdateProjectionMatrix()
    {
        projection = glm::perspective(glm::radians(fov), aspect, clipNear, clipFar);
    }
}