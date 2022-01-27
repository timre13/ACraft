#include "glstuff.h"
#include "Logger.h"
#include "ShaderProg.h"
#include "Texture.h"
#include "Camera.h"
#include "blocks.h"
#include "obj.h"
#include "../deps/OpenSimplexNoise/OpenSimplexNoise/OpenSimplexNoise.h"
#include <cmath>
#include <iomanip>
#include <vector>
#include <ctime>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define WIN_W 1500
#define WIN_H 1000
#define CAM_SPEED 0.1f
#define CAM_FOV_DEG 45.0f

// Number of blocks renderd at once
#define BLOCK_POS_BATCH_SIZE_COUNT 1024*1024*10

#define GROUND_HEIGHT_MAX 300

bool g_isWireframeMode = false;
int g_cursRelativeX = 0;
int g_cursRelativeY = 0;
Camera g_camera = Camera{(float)WIN_W/WIN_H, CAM_FOV_DEG};

bool g_isDebugCam = false;

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

static void toggleDebugCam()
{
    g_isDebugCam = !g_isDebugCam;
}

static void _keyCb(GLFWwindow* win, int key, int scancode, int action, int mods)
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
#define VALS_PER_VERT 5
#define VERT_ATTRIB_INDEX_INST_POS 2

#define BLOCK_VERT_COUNT 36
#define BLOCK_VERT_DATA_LEN (BLOCK_VERT_COUNT * VALS_PER_VERT)
static constexpr float blockVertices[BLOCK_VERT_DATA_LEN] = {
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

    ShaderProg blockShaderProg = ShaderProg{"../src/shaders/block_inst.vert.glsl", "../src/shaders/block_inst.frag.glsl"};
    ShaderProg camModelShaderProg = ShaderProg{"../src/shaders/cam_model.vert.glsl", "../src/shaders/cam_model.frag.glsl"};

    //----------------------------------------------------------------------

    OpenSimplexNoise::Noise noiseGen{std::time(nullptr)};
    std::array<std::vector<glm::vec3>, BLOCK_TYPE__COUNT> blockPositions{};
    for (int x{}; x < 100; ++x)
    {
        for (int z{}; z < 100; ++z)
        {
            const int groundHeight = 50+std::round((noiseGen.eval(x/500.0f, z/500.0f)+0.5f)*(GROUND_HEIGHT_MAX-50));
            for (int y{}; y < GROUND_HEIGHT_MAX; ++y)
            {
                if (y <= groundHeight)
                {
                    // TODO: Dirt blobs inside stone
                    // TODO: More stone types
                    // TODO: Ores
                    const int grassLayerHeight = 1;
                    const int dirtLayerHeight = 5+10*(noiseGen.eval(x/54.0f, z/54.0f)+0.5f);
                    const int stoneLayerHeight = groundHeight*0.75f-20*(noiseGen.eval(x/20.0f, z/20.0f)+0.5f);
                    const int bedrockLayerHeight = 1+2*(noiseGen.eval(x/5.0f, z/5.0f)+0.5f);
                    if (y <= bedrockLayerHeight)
                    {
                        blockPositions[BLOCK_TYPE_BEDROCK].push_back({x, y, z});
                    }
                    else if (y > groundHeight-grassLayerHeight)
                    {
                        blockPositions[BLOCK_TYPE_GRASS].push_back({x, y, z});
                    }
                    else if (y > groundHeight-grassLayerHeight-dirtLayerHeight)
                    {
                        blockPositions[BLOCK_TYPE_DIRT].push_back({x, y, z});
                    }
                    else if (y > groundHeight-grassLayerHeight-dirtLayerHeight-stoneLayerHeight)
                    {
                        blockPositions[BLOCK_TYPE_STONE].push_back({x, y, z});
                    }
                    else
                    {
                        blockPositions[BLOCK_TYPE_DEEPSLATE].push_back({x, y, z});
                    }
                }
            }
        }
    }
    Logger::log << "There are " << blockPositions[BLOCK_TYPE_DIRT].size() << " dirt blocks" << Logger::End;
    Logger::log << "There are " << blockPositions[BLOCK_TYPE_COBBLESTONE].size() << " cobblestone blocks" << Logger::End;
    Logger::log << "There are " << blockPositions[BLOCK_TYPE_GRASS].size() << " grass blocks" << Logger::End;

    //----------------------------------------------------------------------

    Logger::dbg << "Setting up block buffers" << Logger::End;

    uint blockVao{};
    glGenVertexArrays(1, &blockVao);
    glBindVertexArray(blockVao);

    uint blockVbo{};
    uint blockInstVbo{};
    {
        glGenBuffers(1, &blockVbo);

        glBindBuffer(GL_ARRAY_BUFFER, blockVbo);
        glBufferData(GL_ARRAY_BUFFER, BLOCK_VERT_DATA_LEN*sizeof(float), &blockVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(VERT_ATTRIB_INDEX_MESH_COORDS, 3, GL_FLOAT, GL_FALSE, VALS_PER_VERT*sizeof(float), (void*)(0));
        glEnableVertexAttribArray(VERT_ATTRIB_INDEX_MESH_COORDS);

        glVertexAttribPointer(VERT_ATTRIB_INDEX_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, VALS_PER_VERT*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(VERT_ATTRIB_INDEX_TEX_COORDS);

        //----------------------------------------------------------------------

        glGenBuffers(1, &blockInstVbo);

        glBindBuffer(GL_ARRAY_BUFFER, blockInstVbo);
        glBufferData(GL_ARRAY_BUFFER, BLOCK_POS_BATCH_SIZE_COUNT*sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(VERT_ATTRIB_INDEX_INST_POS, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(VERT_ATTRIB_INDEX_INST_POS);
        glVertexAttribDivisor(VERT_ATTRIB_INDEX_INST_POS, 1); // Instanced attribute
    }
    glBindVertexArray(0);

    Logger::dbg << "Set up block buffers" << Logger::End;

    //----------------------------------------------------------------------

    Logger::dbg << "Setting up camera model buffers" << Logger::End;

    auto camModelVertices = loadObjFile("../models/camera.obj");

    uint camModelVao{};
    glGenVertexArrays(1, &camModelVao);
    glBindVertexArray(camModelVao);

    uint camModelVbo{};
    {
        glGenBuffers(1, &camModelVbo);

        glBindBuffer(GL_ARRAY_BUFFER, camModelVbo);
        glBufferData(GL_ARRAY_BUFFER, camModelVertices.size()*sizeof(float), camModelVertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(VERT_ATTRIB_INDEX_MESH_COORDS, 3, GL_FLOAT, GL_FALSE, VALS_PER_VERT*sizeof(float), (void*)(0));
        glEnableVertexAttribArray(VERT_ATTRIB_INDEX_MESH_COORDS);

        glVertexAttribPointer(VERT_ATTRIB_INDEX_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, VALS_PER_VERT*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(VERT_ATTRIB_INDEX_TEX_COORDS);
    }
    glBindVertexArray(0);

    Logger::dbg << "Set up camera model buffers" << Logger::End;

    //----------------------------------------------------------------------

    loadBlockTextures();

    Texture placeholderTex = Texture{"../textures/placeholder.png"};

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

        bool isTitleUpdateNeeded = false;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            g_camera.moveForward(CAM_SPEED, deltaTime);
            isTitleUpdateNeeded = true;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            g_camera.moveBackwards(CAM_SPEED, deltaTime);
            isTitleUpdateNeeded = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            g_camera.moveLeft(CAM_SPEED, deltaTime);
            isTitleUpdateNeeded = true;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            g_camera.moveRight(CAM_SPEED, deltaTime);
            isTitleUpdateNeeded = true;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            g_camera.moveUp(CAM_SPEED, deltaTime);
            isTitleUpdateNeeded = true;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            g_camera.moveDown(CAM_SPEED, deltaTime);
            isTitleUpdateNeeded = true;
        }
        if (isTitleUpdateNeeded)
        {
            glfwSetWindowTitle(window, ("ACraft - Camera: {"
                    +std::to_string(g_camera.getPos().x)+", "
                    +std::to_string(g_camera.getPos().y)+", "
                    +std::to_string(g_camera.getPos().z)+"}").c_str());
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

        //------------------------ Block rendering -----------------------------

        glBindVertexArray(blockVao);
        glBindBuffer(GL_ARRAY_BUFFER, blockInstVbo);
        blockShaderProg.bind();
        g_camera.updateShaderUniforms(blockShaderProg);
        for (int blockType{}; blockType < BLOCK_TYPE__COUNT; ++blockType)
        {
            int renderedBlocks{};
            int remainingBlocks = blockPositions[blockType].size();
            while (remainingBlocks > 0)
            {
                const int batchSize = std::min(BLOCK_POS_BATCH_SIZE_COUNT, remainingBlocks);

                glBufferSubData(GL_ARRAY_BUFFER, 0, batchSize*sizeof(glm::vec3), blockPositions[blockType].data()+renderedBlocks);
                blockTextures[blockType].bind();
                glDrawArraysInstanced(GL_TRIANGLES, 0, BLOCK_VERT_COUNT, batchSize);
                remainingBlocks -= BLOCK_POS_BATCH_SIZE_COUNT;
                renderedBlocks += BLOCK_POS_BATCH_SIZE_COUNT;
            }
        }

        //------------------- Debug camera model rendering ---------------------

        if (g_isDebugCam)
        {
            auto camModelMat = glm::mat4(1.0f);
            camModelMat = glm::translate(camModelMat, {g_camera.getPos().x, g_camera.getPos().y-10, g_camera.getPos().z});
            camModelMat = glm::rotate(camModelMat, glm::radians(g_camera.getHorizRotDeg()+90.0f), {0.0f, -1.0f, 0.0f});
            camModelMat = glm::rotate(camModelMat, glm::radians(g_camera.getVertRotDeg()), {1.0f, 0.0f, 0.0f});
            camModelMat = glm::scale(camModelMat, {10.0f, 10.0f, 10.0f});

            glBindVertexArray(camModelVao);
            camModelShaderProg.bind();
            camModelShaderProg.setUniform("inModelMat", camModelMat);
            g_camera.updateShaderUniforms(camModelShaderProg);
            placeholderTex.bind();
            glDrawArrays(GL_TRIANGLES, 0, camModelVertices.size()/VALS_PER_VERT);
        }

        //----------------------------------------------------------------------

        glfwSwapBuffers(window);
    }

    Logger::log << "Cleaning up" << Logger::End;
    glDeleteBuffers(1, &blockVbo);
    glDeleteBuffers(1, &blockInstVbo);
    glDeleteVertexArrays(1, &blockVao);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
