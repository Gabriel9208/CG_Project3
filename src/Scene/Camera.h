#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace CG
{
    class Camera
    {
    public:
        Camera();
        ~Camera();

        void LookAt();
        void LookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up);

        auto GetProjectionMatrix() -> const glm::mat4*
        {
            return &projection;
        }

        auto GetViewMatrix() -> const glm::mat4*
        {
            return &view;
        }

        void SetFov(float degree)
        {
            fov = degree;

            UpdateProjectionMatrix();
        }

        void SetAspect(float ratio)
        {
            aspect = ratio;

            UpdateProjectionMatrix();
        }

        void SetClip(float near, float far)
        {
            clipNear = near;
            clipFar = far;

            UpdateProjectionMatrix();
        }

        void rotateAround(float angle, glm::vec3 axis);

        void flatTranslate(float wRight, float wUp);
        void zoom(float weight);

        inline glm::vec3 getPos() const { return pos; }
        inline glm::vec3 getTarget() const { return target; }
        inline glm::vec3 getUp() const { return up; }

    private:
        void UpdateProjectionMatrix();
        void updateVectors(const glm::quat deltaOrientation);

        void setPos(glm::vec3 newPos);
        void setTarget(glm::vec3 newTarget);
        void setUpVector(glm::vec3 newUp);

    private:
        glm::mat4 projection;
        glm::mat4 view;

        glm::vec3 pos;
        glm::vec3 target;
        glm::vec3 up;

        float fov;
        float aspect;
        float clipNear;
        float clipFar;
    };
}
