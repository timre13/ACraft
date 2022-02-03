#pragma once

#include "glstuff.h"

void GLAPIENTRY _glMsgCb(
        GLenum source, GLenum type, GLuint, GLenum severity,
        GLsizei, const GLchar* message, const void*);

void _glfwErrCb(int err, const char* desc);

void _windowCloseCb(GLFWwindow*);
void _windowResizeCb(GLFWwindow*, int width, int height);
void _keyCb(GLFWwindow* win, int key, int scancode, int action, int mods);
void toggleWireframeMode();
void toggleDebugCam();
void _mouseMoveCb(GLFWwindow*, double x, double y);
