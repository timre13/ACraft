#pragma once

#include <glm/vec3.hpp>
#include "ShaderProg.h"

class Camera final
{
private:
    template <typename T>
    class MaybeOutdated final
    {
    private:
        T m_value{};
        bool m_isOutdated = true;

    public:
        inline MaybeOutdated() = default;
        inline MaybeOutdated(const T& val)
            : m_value{val}, m_isOutdated{false}
        {
        }

        inline void set(const T& val)
        {
            m_value = val;
            m_isOutdated = false;
        }

        inline void markOutdated() { m_isOutdated = true; }
        inline bool isOutdated() const { return m_isOutdated; }
        inline const T& get() const { return m_value; }
    };

    glm::vec3 m_pos{};
    float m_fovDeg{};
    float m_winAspectRatio{};
    glm::vec3 m_frontVec{};
    float m_yawDeg{}; // Horizontal rotation
    float m_pitchDeg{}; // Vertical rotation
    float m_near = 0.01f;
    float m_far = 1000.0f;

    MaybeOutdated<glm::mat4> m_viewMat{};
    MaybeOutdated<glm::mat4> m_projMat{};
    MaybeOutdated<glm::mat4> m_frustumMat{};

    void _recalcProjMat();
    void _recalcViewMat();
    void _recalcFrustumMat();

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

    inline const glm::vec3& getPos() const { return m_pos; }
    inline float getHorizRotDeg() const { return m_yawDeg; }
    inline float getVertRotDeg() const { return m_pitchDeg; }
    inline const glm::vec3& getFrontVec() const { return m_frontVec; }

    void updateShaderUniformsIfNeeded(ShaderProg& shader);

    void onDebugModeSwitch();

    bool isPointVisible(const glm::vec3& point);
};
