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
    float m_yawDeg{}; // Horizontal rotation
    float m_pitchDeg{}; // Vertical rotation

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

    inline glm::vec3 getPos() const { return m_pos; }
    inline float getHorizRotDeg() const { return m_yawDeg; }
    inline float getVertRotDeg() const { return m_pitchDeg; }

    void updateShaderUniforms(ShaderProg& shader);
};
