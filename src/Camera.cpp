#include "Camera.h"

void Camera::recalcFrontVec()
{
    glm::vec3 camDir;
    camDir.x = cos(glm::radians(m_yawDeg)) * cos(glm::radians(m_pitchDeg));
    camDir.y = sin(glm::radians(m_pitchDeg));
    camDir.z = sin(glm::radians(m_yawDeg)) * cos(glm::radians(m_pitchDeg));
    m_frontVec = glm::normalize(camDir);
}

Camera::Camera(float winAspectRatio, float fovDeg)
{
    setWinAspectRatio(winAspectRatio);
    setFovDeg(fovDeg);
    recalcFrontVec();
}

void Camera::setFovDeg(float deg)
{
    m_fovDeg = deg;
}

void Camera::setWinAspectRatio(float ratio)
{
    m_winAspectRatio = ratio;
}

void Camera::setPos(const glm::vec3& pos)
{
    m_pos = pos;
}

void Camera::moveForward(float amount, int frameTime)
{
    const float prevY = m_pos.y;
    m_pos += m_frontVec * (amount * frameTime);
    m_pos.y = prevY;
}

void Camera::moveBackwards(float amount, int frameTime)
{
    const float prevY = m_pos.y;
    m_pos -= m_frontVec * (amount * frameTime);
    m_pos.y = prevY;
}

void Camera::moveLeft(float amount, int frameTime)
{
    const float prevY = m_pos.y;
    m_pos -= glm::normalize(glm::cross(m_frontVec, {0.0f, 1.0f, 0.0f})) * (amount * frameTime);
    m_pos.y = prevY;
}

void Camera::moveRight(float amount, int frameTime)
{
    const float prevY = m_pos.y;
    m_pos += glm::normalize(glm::cross(m_frontVec, {0.0f, 1.0f, 0.0f})) * (amount * frameTime);
    m_pos.y = prevY;
}

void Camera::moveUp(float amount, int frameTime)
{
    m_pos.y += (amount * frameTime);
}

void Camera::moveDown(float amount, int frameTime)
{
    m_pos.y -= (amount * frameTime);
}

void Camera::rotateHorizontallyDeg(float deg)
{
    m_yawDeg += deg;

    recalcFrontVec();
}

void Camera::rotateVerticallyDeg(float deg)
{
    m_pitchDeg += deg;
    if (m_pitchDeg > 89.9)
        m_pitchDeg = 89.9;
    else if (m_pitchDeg < -89.9)
        m_pitchDeg = -89.9;

    recalcFrontVec();
}

void Camera::updateShaderUniforms(ShaderProg& shader)
{
    auto viewMat = glm::lookAt(m_pos, m_pos+m_frontVec, {0.0f, 1.0f, 0.0f});
    shader.setUniform("inViewMat", viewMat);

    auto projMat = glm::perspective(glm::radians(m_fovDeg), m_winAspectRatio, 0.1f, 10000.0f);
    shader.setUniform("inProjMat", projMat);
}
