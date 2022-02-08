#include "Camera.h"
#include "Logger.h"
#include <cmath>

extern bool g_isDebugCam;

void Camera::_recalcProjMat()
{
    m_projMat = glm::perspective(glm::radians(m_fovDeg), m_winAspectRatio, m_near, m_far);

    m_frustumMat.markOutdated();
}

void Camera::_recalcViewMat()
{
    glm::vec3 camDir;
    camDir.x = cos(glm::radians(m_yawDeg)) * cos(glm::radians(m_pitchDeg));
    camDir.y = sin(glm::radians(m_pitchDeg));
    camDir.z = sin(glm::radians(m_yawDeg)) * cos(glm::radians(m_pitchDeg));
    m_frontVec = glm::normalize(camDir);

    if (g_isDebugCam)
    {
        static constexpr auto pos = glm::vec3{0.0f, 1000.0f, 0.0f};
        m_viewMat = glm::lookAt(pos, glm::vec3{25.0f, 0.0f, 25.0f}, {0.0f, 1.0f, 0.0f});
    }
    else
    {
        m_viewMat = glm::lookAt(m_pos, m_pos+m_frontVec, {0.0f, 1.0f, 0.0f});
    }

    m_frustumMat.markOutdated();
}

void Camera::_recalcFrustumMat()
{
    m_frustumMat = m_projMat.get() * m_viewMat.get();
}

Camera::Camera(float winAspectRatio, float fovDeg)
{
    setWinAspectRatio(winAspectRatio);
    setFovDeg(fovDeg);
    m_projMat.markOutdated();
    m_viewMat.markOutdated();
}

void Camera::setFovDeg(float deg)
{
    m_fovDeg = deg;
    m_projMat.markOutdated();
}

void Camera::setWinAspectRatio(float ratio)
{
    m_winAspectRatio = ratio;
    m_projMat.markOutdated();
}

void Camera::setPos(const glm::vec3& pos)
{
    m_pos = pos;
    m_viewMat.markOutdated();
}

void Camera::moveForward(float amount, int frameTime)
{
    const float prevY = m_pos.y;
    m_pos += m_frontVec * (amount * frameTime);
    m_pos.y = prevY;
    m_viewMat.markOutdated();
}

void Camera::moveBackwards(float amount, int frameTime)
{
    const float prevY = m_pos.y;
    m_pos -= m_frontVec * (amount * frameTime);
    m_pos.y = prevY;
    m_viewMat.markOutdated();
}

void Camera::moveLeft(float amount, int frameTime)
{
    const float prevY = m_pos.y;
    m_pos -= glm::normalize(glm::cross(m_frontVec, {0.0f, 1.0f, 0.0f})) * (amount * frameTime);
    m_pos.y = prevY;
    m_viewMat.markOutdated();
}

void Camera::moveRight(float amount, int frameTime)
{
    const float prevY = m_pos.y;
    m_pos += glm::normalize(glm::cross(m_frontVec, {0.0f, 1.0f, 0.0f})) * (amount * frameTime);
    m_pos.y = prevY;
    m_viewMat.markOutdated();
}

void Camera::moveUp(float amount, int frameTime)
{
    m_pos.y += (amount * frameTime);
    m_viewMat.markOutdated();
}

void Camera::moveDown(float amount, int frameTime)
{
    m_pos.y -= (amount * frameTime);
    m_viewMat.markOutdated();
}

void Camera::rotateHorizontallyDeg(float deg)
{
    m_yawDeg += deg;
    m_viewMat.markOutdated();
}

void Camera::rotateVerticallyDeg(float deg)
{
    m_pitchDeg += deg;
    if (m_pitchDeg > 89.9)
        m_pitchDeg = 89.9;
    else if (m_pitchDeg < -89.9)
        m_pitchDeg = -89.9;

    m_viewMat.markOutdated();
}

void Camera::updateShaderUniformsIfNeeded(ShaderProg& shader)
{
    if (m_viewMat.isOutdated())
    {
        _recalcViewMat();
        shader.setUniform("inViewMat", m_viewMat.get());
        m_frustumMat.markOutdated();
    }

    if (m_projMat.isOutdated())
    {
        _recalcProjMat();
        shader.setUniform("inProjMat", m_projMat.get());
        m_frustumMat.markOutdated();
    }

    if (m_frustumMat.isOutdated())
    {
        _recalcFrustumMat();
    }
}

void Camera::onDebugModeSwitch()
{
    m_viewMat.markOutdated();
}

bool Camera::isPointVisible(const glm::vec3& point)
{
    // See: http://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf

    const glm::vec4 vector = glm::vec4(point, 1);
    const glm::vec4 transfVec = m_frustumMat.get() * vector;

    return (-transfVec.w < transfVec.x < transfVec.w)
       &&  (-transfVec.w < transfVec.y < transfVec.w)
       &&  (-transfVec.w < transfVec.z < transfVec.w);
}
