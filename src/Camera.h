#pragma once

#include <glm/vec3.hpp>
#include "ShaderProg.h"

class Camera final
{
private:
    glm::vec3 m_pos{};
    float m_fovDeg{};
    float m_winAspectRatio{};
    glm::vec3 m_frontVec{};
    float m_yawDeg{-90.0f};
    float m_pitchDeg{};

    void recalcFrontVec();

public:
    Camera(float winAspectRatio, float fovDeg);

    void setFovDeg(float deg);
    void setWinAspectRatio(float ratio);
    void setPos(const glm::vec3& pos);

    void moveForward(float amount, int frameTime);
    void moveBackwards(float amount, int frameTime);
    void moveLeft(float amount, int frameTime);
    void moveRight(float amount, int frameTime);
    void moveUp(float amount, int frameTime);
    void moveDown(float amount, int frameTime);

    void rotateHorizontallyDeg(float deg);
    void rotateVerticallyDeg(float deg);

    void updateShaderUniforms(ShaderProg& shader);
};
