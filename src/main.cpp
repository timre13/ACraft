#include "glstuff.h"
#include "Logger.h"
#include "ShaderProg.h"
#include "Texture.h"
#include "Camera.h"
#include "../deps/OpenSimplexNoise/OpenSimplexNoise/OpenSimplexNoise.h"
#include <cmath>
#include <iomanip>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define WIN_W 1500
#define WIN_H 1000
#define CAM_SPEED 0.1f
#define CAM_FOV_DEG 45.0f

bool g_isWireframeMode = false;
int g_cursRelativeX = 0;
int g_cursRelativeY = 0;
Camera g_camera = Camera{(float)WIN_W/WIN_H, CAM_FOV_DEG};

static void _glfwErrCb(int err, const char* desc)
{
    Logger::fatal << "GLFW Error: " << std::hex << err << std::dec << ": " << desc << Logger::End;
}

static void GLAPIENTRY _glMsgCb(
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
}

static void _windowCloseCb(GLFWwindow*)
{
    Logger::log << "Closing window" << Logger::End;
}

static void _windowResizeCb(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
    //Logger::dbg << "Resized window to " << width << 'x' << height << Logger::End;
    g_camera.setWinAspectRatio((float)width/height);
}

static void toggleWireframeMode()
{
    g_isWireframeMode = !g_isWireframeMode;
    glPolygonMode(GL_FRONT_AND_BACK, g_isWireframeMode ? GL_LINE : GL_FILL);
}

static void _keyCb(GLFWwindow* win, int key, int scancode, int action, int mods)
{
    (void)win;
    (void)scancode;

    if (action == GLFW_PRESS && mods == 0 && key == GLFW_KEY_F3)
    {
        toggleWireframeMode();
    }
}

static void _mouseMoveCb(GLFWwindow*, double x, double y)
{
    static double cursLastX = x;
    static double cursLastY = y;

    g_cursRelativeX = x - cursLastX;
    g_cursRelativeY = cursLastY - y;
    cursLastX = x;
    cursLastY = y;
}

#define VERT_ATTRIB_INDEX_MESH_COORDS 0
#define VERT_ATTRIB_INDEX_TEX_COORDS 1
#define VERT_ATTRIB_INDEX_INST_MAT_0 2
#define VERT_ATTRIB_INDEX_INST_MAT_1 3
#define VERT_ATTRIB_INDEX_INST_MAT_2 4
#define VERT_ATTRIB_INDEX_INST_MAT_3 5
#define CUBE_VERT_COUNT 36
#define CUBE_VALS_PER_VERT 5
#define CUBE_VERT_DATA_LEN (CUBE_VERT_COUNT * CUBE_VALS_PER_VERT)
static constexpr float cubeVertices[CUBE_VERT_DATA_LEN] = {
    //       Mesh              Texture
    // X      Y      Z          X     Y
    -1.0f,  1.0f, -1.0f, /**/ 0.0f,  1.0f,
     1.0f,  1.0f,  1.0f, /**/ 1.0f,  0.0f,
     1.0f,  1.0f, -1.0f, /**/ 1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, /**/ 1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f, /**/ 0.0f,  0.0f,
     1.0f, -1.0f,  1.0f, /**/ 1.0f,  0.0f,
    -1.0f,  1.0f,  1.0f, /**/ 0.0f,  1.0f,
    -1.0f, -1.0f, -1.0f, /**/ 1.0f,  0.0f,
    -1.0f, -1.0f,  1.0f, /**/ 0.0f,  0.0f,
     1.0f, -1.0f, -1.0f, /**/ 1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f, /**/ 0.0f,  0.0f,
    -1.0f, -1.0f, -1.0f, /**/ 0.0f,  1.0f,
     1.0f,  1.0f, -1.0f, /**/ 1.0f,  1.0f,
     1.0f, -1.0f,  1.0f, /**/ 0.0f,  0.0f,
     1.0f, -1.0f, -1.0f, /**/ 1.0f,  0.0f,
    -1.0f,  1.0f, -1.0f, /**/ 0.0f,  1.0f,
     1.0f, -1.0f, -1.0f, /**/ 1.0f,  0.0f,
    -1.0f, -1.0f, -1.0f, /**/ 0.0f,  0.0f,
    -1.0f,  1.0f, -1.0f, /**/ 0.0f,  1.0f,
    -1.0f,  1.0f,  1.0f, /**/ 0.0f,  0.0f,
     1.0f,  1.0f,  1.0f, /**/ 1.0f,  0.0f,
     1.0f,  1.0f,  1.0f, /**/ 1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f, /**/ 0.0f,  1.0f,
    -1.0f, -1.0f,  1.0f, /**/ 0.0f,  0.0f,
    -1.0f,  1.0f,  1.0f, /**/ 0.0f,  1.0f,
    -1.0f,  1.0f, -1.0f, /**/ 1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f, /**/ 1.0f,  0.0f,
     1.0f, -1.0f, -1.0f, /**/ 1.0f,  1.0f,
     1.0f, -1.0f,  1.0f, /**/ 1.0f,  0.0f,
    -1.0f, -1.0f,  1.0f, /**/ 0.0f,  0.0f,
     1.0f,  1.0f, -1.0f, /**/ 1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, /**/ 0.0f,  1.0f,
     1.0f, -1.0f,  1.0f, /**/ 0.0f,  0.0f,
    -1.0f,  1.0f, -1.0f, /**/ 0.0f,  1.0f,
     1.0f,  1.0f, -1.0f, /**/ 1.0f,  1.0f,
     1.0f, -1.0f, -1.0f, /**/ 1.0f,  0.0f,
};
#define BLOCK_MODEL_SCALE 0.5f

int main()
{
    glfwSetErrorCallback(_glfwErrCb);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(WIN_W, WIN_H, "ACraft", NULL, NULL);
    glfwSetWindowCloseCallback(window, _windowCloseCb);
    glfwSetWindowSizeCallback(window, _windowResizeCb);
    glfwSetKeyCallback(window, _keyCb);
    glfwSetCursorPosCallback(window, _mouseMoveCb);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);

    glewExperimental = true;
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK)
    {
        Logger::fatal << "Failed to initialize GLEW: " << glewGetErrorString(glewErr) << Logger::End;
    }

    Logger::dbg << "-------------------- OpenGL Info --------------------\n";
    Logger::dbg << "Vendor:         " << glGetString(GL_VENDOR) << '\n';
    Logger::dbg << "Renderer:       " << glGetString(GL_RENDERER) << '\n';
    Logger::dbg << "Version:        " << glGetString(GL_VERSION) << '\n';
    Logger::dbg << "GLSL version:   " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';
#if 0
    int extCount{};
    glGetIntegerv(GL_NUM_EXTENSIONS, &extCount);
    Logger::dbg << "Extensions (" << extCount << "): ";
    for (int i{}; i < extCount; ++i)
    {
        Logger::dbg << glGetStringi(GL_EXTENSIONS, i) << ' ';
    }
#endif
    Logger::dbg << Logger::End;

    //----------------------------------------------------------------------

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(_glMsgCb, 0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    //----------------------------------------------------------------------

    ShaderProg mainShaderProg = ShaderProg{"../src/shaders/main.vert.glsl", "../src/shaders/main.frag.glsl"};

    //----------------------------------------------------------------------

    OpenSimplexNoise::Noise noiseGen;
    std::vector<glm::vec3> blockPositions{};
    for (int x{}; x < 1000; ++x)
    {
        for (int z{}; z < 1000; ++z)
        {
            const int groundHeight = std::round((noiseGen.eval(x/500.0f, z/500.0f)+0.5f)*100);
            for (int y{}; y < 100; ++y)
            {
                if (y <= groundHeight && y > groundHeight-3)
                {
                    blockPositions.push_back({x, y, z});
                }
            }
        }
    }

    //----------------------------------------------------------------------

    uint cubeVao{};
    glGenVertexArrays(1, &cubeVao);
    glBindVertexArray(cubeVao);

    uint cubeVbo{};
    uint instancedVbo{};
    {
        glGenBuffers(1, &cubeVbo);

        glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
        glBufferData(GL_ARRAY_BUFFER, CUBE_VERT_DATA_LEN*sizeof(float), &cubeVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(VERT_ATTRIB_INDEX_MESH_COORDS, 3, GL_FLOAT, GL_FALSE, CUBE_VALS_PER_VERT*sizeof(float), (void*)(0));
        glEnableVertexAttribArray(VERT_ATTRIB_INDEX_MESH_COORDS);

        glVertexAttribPointer(VERT_ATTRIB_INDEX_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, CUBE_VALS_PER_VERT*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(VERT_ATTRIB_INDEX_TEX_COORDS);

        //----------------------------------------------------------------------

        glGenBuffers(1, &instancedVbo);

        glBindBuffer(GL_ARRAY_BUFFER, instancedVbo);
        glBufferData(GL_ARRAY_BUFFER, blockPositions.size()*sizeof(glm::vec3), blockPositions.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(VERT_ATTRIB_INDEX_INST_MAT_0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(VERT_ATTRIB_INDEX_INST_MAT_0);
        glVertexAttribDivisor(VERT_ATTRIB_INDEX_INST_MAT_0, 1); // Instanced attribute
    }
    glBindVertexArray(0);

    //----------------------------------------------------------------------

    Texture cobbleStoneTex = Texture{"../textures/cobblestone.png"};

    //----------------------------------------------------------------------

    g_camera.setPos({0.0f, 5.0f, 0.0f});

    double lastTime{};
    glfwSwapInterval(1); // Force V-Sync
    while (!glfwWindowShouldClose(window))
    {
        const double currTime = glfwGetTime()*1000;
        const double deltaTime = currTime - lastTime;
        lastTime = currTime;

        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            g_camera.moveForward(CAM_SPEED, deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            g_camera.moveBackwards(CAM_SPEED, deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            g_camera.moveLeft(CAM_SPEED, deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            g_camera.moveRight(CAM_SPEED, deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            g_camera.moveUp(CAM_SPEED, deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            g_camera.moveDown(CAM_SPEED, deltaTime);
        }

        glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (g_cursRelativeX)
        {
            g_camera.rotateHorizontallyDeg(g_cursRelativeX/10.0f);
            g_cursRelativeX = 0;
        }
        if (g_cursRelativeY)
        {
            g_camera.rotateVerticallyDeg(g_cursRelativeY/10.0f);
            g_cursRelativeY = 0;
        }

        glBindVertexArray(cubeVao);
        mainShaderProg.bind();
        g_camera.updateShaderUniforms(mainShaderProg);
        cobbleStoneTex.bind();
        glDrawArraysInstanced(GL_TRIANGLES, 0, CUBE_VERT_COUNT, blockPositions.size());

        glfwSwapBuffers(window);
    }

    Logger::log << "Cleaning up" << Logger::End;
    glDeleteBuffers(1, &cubeVbo);
    glDeleteBuffers(1, &instancedVbo);
    glDeleteVertexArrays(1, &cubeVao);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
