#include "callbacks.h"
#include "Logger.h"
#include "Camera.h"

extern bool g_isWireframeMode;
extern int g_cursRelativeX;
extern int g_cursRelativeY;
extern bool g_isDebugCam;
extern Camera g_camera;

void GLAPIENTRY _glMsgCb(
        GLenum source, GLenum type, GLuint, GLenum severity,
        GLsizei, const GLchar* message, const void*)
{
    std::cout << "\033[93m[OpenGL]\033[0m: " << "Message: type=\"";
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR: std::cout << "other"; break;
    case GL_DEBUG_TYPE_PERFORMANCE: std::cout << "performance"; break;
    case GL_DEBUG_TYPE_PORTABILITY: std::cout << "portability"; break;
    case GL_DEBUG_TYPE_POP_GROUP: std::cout << "pop group"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP: std::cout << "push group"; break;
    case GL_DEBUG_TYPE_MARKER: std::cout << "marker"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: std::cout << "undefined behavior"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "deprecated behavior"; break;
    default: std::cout << "unknown"; break;
    }

    std::cout << "\", source=\"";
    switch (source)
    {
    case GL_DEBUG_SOURCE_API: std::cout << "API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: std::cout << "window system"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "shader compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY: std::cout << "third party"; break;
    case GL_DEBUG_SOURCE_APPLICATION: std::cout << "application"; break;
    case GL_DEBUG_SOURCE_OTHER: std::cout << "other"; break;
    default: std::cout << "unknown"; break;
    }

    std::cout << "\", severity=\"";
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH: std::cout << "high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM: std::cout << "medium"; break;
    case GL_DEBUG_SEVERITY_LOW: std::cout << "low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "notification"; break;
    default: std::cout << "unknown"; break;
    }

    std::cout << "\" : " << message << '\n';
    std::cout.flush();

    int arrayBufferBinding{};
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &arrayBufferBinding);
    int vertexArrayBinding{};
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertexArrayBinding);
    int tex2dBinding{};
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex2dBinding);
    int tex2dArrBinding{};
    glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &tex2dArrBinding);
    Logger::dbg << "State: "
        << "\n\tBound VAO              = " << vertexArrayBinding
        << "\n\tBound VBO              = " << arrayBufferBinding
        << "\n\tBound 2D Texture       = " << tex2dBinding
        << "\n\tBound 2D Texture Array = " << tex2dArrBinding
        << Logger::End;
}

void _glfwErrCb(int err, const char* desc)
{
    Logger::fatal << "GLFW Error: " << std::hex << err << std::dec << ": " << desc << Logger::End;
}

void _windowCloseCb(GLFWwindow*)
{
    Logger::log << "Closing window" << Logger::End;
}

void _windowResizeCb(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
    //Logger::dbg << "Resized window to " << width << 'x' << height << Logger::End;
    g_camera.setWinAspectRatio((float)width/height);
}

void _keyCb(GLFWwindow* win, int key, int scancode, int action, int mods)
{
    (void)win;
    (void)scancode;

    if (action == GLFW_PRESS && mods == 0)
    {
        if (key == GLFW_KEY_F3)
        {
            toggleWireframeMode();
        }
        else if (key == GLFW_KEY_Q)
        {
            toggleDebugCam();
        }
    }
}

void toggleWireframeMode()
{
    g_isWireframeMode = !g_isWireframeMode;
    glPolygonMode(GL_FRONT_AND_BACK, g_isWireframeMode ? GL_LINE : GL_FILL);
}

void toggleDebugCam()
{
    g_isDebugCam = !g_isDebugCam;
}

void _mouseMoveCb(GLFWwindow*, double x, double y)
{
    static double cursLastX = x;
    static double cursLastY = y;

    g_cursRelativeX = x - cursLastX;
    g_cursRelativeY = cursLastY - y;
    cursLastX = x;
    cursLastY = y;
}
