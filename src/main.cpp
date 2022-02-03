#include "glstuff.h"
#include "Logger.h"
#include "ShaderProg.h"
#include "Texture.h"
#include "Camera.h"
#include "blocks.h"
#include "obj.h"
#include "callbacks.h"
#include "../deps/OpenSimplexNoise/OpenSimplexNoise/OpenSimplexNoise.h"
#include "../deps/stb/stb_image.h"
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

#define GROUND_HEIGHT_MAX 384

#define CHUNK_WIDTH_BLOCKS 16

bool g_isWireframeMode = false;
int g_cursRelativeX = 0;
int g_cursRelativeY = 0;
auto g_camera = Camera{(float)WIN_W/WIN_H, CAM_FOV_DEG};
auto g_noiseGen = OpenSimplexNoise::Noise{std::time(nullptr)};

bool g_isDebugCam = false;


struct Block
{
    BlockType type{};
};

#define VERT_ATTRIB_INDEX_MESH_COORDS 0
#define VERT_ATTRIB_INDEX_TEX_COORDS 1
#define VALS_PER_VERT 5
#define VERT_ATTRIB_INDEX_INST_POS 2
#define VERT_ATTRIB_INDEX_INST_TYPE 3

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

struct Chunk
{
    /*
     * Position of the chunk in a chunk-sized grid.
     */
    int chunkX{};
    int chunkZ{};

    using BlockRow_t = std::array<Block, CHUNK_WIDTH_BLOCKS>;
    using ChunkSlice_t = std::array<BlockRow_t, CHUNK_WIDTH_BLOCKS>;
    using ChunkContent_t = std::array<ChunkSlice_t, GROUND_HEIGHT_MAX>;
    ChunkContent_t blocks; // Indexing: [y][z][x]
};

Chunk genChunk(int chunkX, int chunkZ)
{
    Chunk chunk;
    chunk.chunkX = chunkX;
    chunk.chunkZ = chunkZ;

    for (int offsX{}; offsX < CHUNK_WIDTH_BLOCKS; ++offsX)
    {
        for (int offsZ{}; offsZ < CHUNK_WIDTH_BLOCKS; ++offsZ)
        {
            const int x = chunkX*CHUNK_WIDTH_BLOCKS+offsX;
            const int z = chunkZ*CHUNK_WIDTH_BLOCKS+offsZ;

            const int groundHeight = 50+std::round((g_noiseGen.eval(x/500.0f, z/500.0f)+0.5f)*(GROUND_HEIGHT_MAX-50));
            for (int y{}; y < GROUND_HEIGHT_MAX; ++y)
            {
                BlockType type = BLOCK_TYPE_AIR;
                if (y <= groundHeight)
                {
                    // TODO: More stone types
                    // TODO: Ores
                    const int grassLayerHeight = 1;
                    const int dirtLayerHeight = 5+10*(g_noiseGen.eval(x/54.0f, z/54.0f)+0.5f);
                    const int stoneLayerHeight = groundHeight*0.75f-20*(g_noiseGen.eval(x/20.0f, z/20.0f)+0.5f);
                    const int bedrockLayerHeight = 1+2*(g_noiseGen.eval(x/5.0f, z/5.0f)+0.5f);
                    const int isDirtBlob = y > 20 && g_noiseGen.eval(x/8.0f, z/8.0f, y/8.0f) >= 0.4f;
                    const int isCoalOreBlob = g_noiseGen.eval(x/7.0f+10, z/7.0f+10, y/7.0f+10) >= 0.6f; // Coal or deepslate coal
                    if (y <= bedrockLayerHeight)
                    {
                        type = BLOCK_TYPE_BEDROCK;
                    }
                    else if (y > groundHeight-grassLayerHeight)
                    {
                        type = BLOCK_TYPE_GRASS;
                    }
                    else if (isDirtBlob || y > groundHeight-grassLayerHeight-dirtLayerHeight)
                    {
                        type = BLOCK_TYPE_DIRT;
                    }
                    else if (y > groundHeight-grassLayerHeight-dirtLayerHeight-stoneLayerHeight)
                    {
                        if (isCoalOreBlob)
                        {
                            type = BLOCK_TYPE_COAL_ORE;
                        }
                        else
                        {
                            type = BLOCK_TYPE_STONE;
                        }
                    }
                    else
                    {
                        if (isCoalOreBlob)
                        {
                            type = BLOCK_TYPE_DEEPSLATE_COAL_ORE;
                        }
                        else
                        {
                            type = BLOCK_TYPE_DEEPSLATE;
                        }
                    }
                }
                chunk.blocks[y][offsZ][offsX].type = type;
            }
        }
    }
    return chunk;
}

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

    std::vector<Chunk> chunks;
    chunks.push_back(genChunk(0, 0));

    //----------------------------------------------------------------------

    Logger::dbg << "Setting up block buffers" << Logger::End;

    uint blockVao{};
    glGenVertexArrays(1, &blockVao);
    glBindVertexArray(blockVao);

    uint blockVbo{};
    uint blockPosInstVbo{};
    uint blockTypeInstVbo{};
    {
        glGenBuffers(1, &blockVbo);

        glBindBuffer(GL_ARRAY_BUFFER, blockVbo);
        glBufferData(GL_ARRAY_BUFFER, BLOCK_VERT_DATA_LEN*sizeof(float), &blockVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(VERT_ATTRIB_INDEX_MESH_COORDS, 3, GL_FLOAT, GL_FALSE, VALS_PER_VERT*sizeof(float), (void*)(0));
        glEnableVertexAttribArray(VERT_ATTRIB_INDEX_MESH_COORDS);

        glVertexAttribPointer(VERT_ATTRIB_INDEX_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, VALS_PER_VERT*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(VERT_ATTRIB_INDEX_TEX_COORDS);

        //----------------------------------------------------------------------

        glGenBuffers(1, &blockPosInstVbo);

        glBindBuffer(GL_ARRAY_BUFFER, blockPosInstVbo);
        glBufferData(GL_ARRAY_BUFFER, BLOCK_POS_BATCH_SIZE_COUNT*sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(VERT_ATTRIB_INDEX_INST_POS, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(VERT_ATTRIB_INDEX_INST_POS);
        glVertexAttribDivisor(VERT_ATTRIB_INDEX_INST_POS, 1); // Instanced attribute


        glGenBuffers(1, &blockTypeInstVbo);

        glBindBuffer(GL_ARRAY_BUFFER, blockTypeInstVbo);
        glBufferData(GL_ARRAY_BUFFER, BLOCK_POS_BATCH_SIZE_COUNT*sizeof(float), nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(VERT_ATTRIB_INDEX_INST_TYPE, 1, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(VERT_ATTRIB_INDEX_INST_TYPE);
        glVertexAttribDivisor(VERT_ATTRIB_INDEX_INST_TYPE, 1); // Instanced attribute
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

    int maxArrayTextureLayers{};
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxArrayTextureLayers);
    Logger::dbg << "GL_MAX_ARRAY_TEXTURE_LAYERS = " << maxArrayTextureLayers << Logger::End;
    assert(BLOCK_TYPE__COUNT <= maxArrayTextureLayers); // Check if we have space to store all the textures in a texture array

    uint blockTexArray{};
    glGenTextures(1, &blockTexArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, blockTexArray);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 16, 16, BLOCK_TYPE__COUNT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    // Fill each image layer with the data
    // Note: Skip air
    for (size_t i{1}; i < (size_t)BLOCK_TYPE__COUNT; ++i)
    {
        std::string path = std::string(BLOCK_TEXTURE_DIR) + "/" + blockTexturePaths[i];

        int width{};
        int height{};
        int _{};
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &_, 4);
        assert(width == 16);
        assert(height == 16);

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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

        // TODO: Only render visible blocks:
        //  * https://community.khronos.org/t/improve-performance-render-100000-objects/67088/3

        //------------------------ Block rendering -----------------------------

        std::vector<glm::vec3> blockPositions{};
        blockPositions.reserve(10000);
        std::vector<int> blockTexIds{};
        blockTexIds.reserve(10000);
        // Prepare block data
        for (const auto& chunk : chunks)
        {
            // TODO: Check for chunk visibility
            {
                for (int sliceI{}; sliceI < (int)chunk.blocks.size(); ++sliceI) // Y
                {
                    const auto& slice = chunk.blocks[sliceI];

                    for (int rowI{}; rowI < (int)slice.size(); ++rowI) // Z
                    {
                        const auto& row = slice[rowI];

                        for (int blockI{}; blockI < (int)row.size(); ++blockI) // X
                        {
                            const auto& block = row[blockI];

                            // Don't render air
                            if (block.type == BLOCK_TYPE_AIR)
                                continue;

                            // TODO: Check for block visibility

                            blockTexIds.push_back(block.type);
                            blockPositions.push_back({
                                    (chunk.chunkX*CHUNK_WIDTH_BLOCKS+blockI),
                                    sliceI,
                                    (chunk.chunkZ*CHUNK_WIDTH_BLOCKS+rowI)
                            });
                        }
                    }
                }
            }
        }

        // Render blocks
        {
            glBindVertexArray(blockVao);
            blockShaderProg.bind();
            g_camera.updateShaderUniforms(blockShaderProg);

            int renderedBlocks{};
            int remainingBlocks = blockPositions.size();
            while (remainingBlocks > 0)
            {
                const int batchSize = std::min(BLOCK_POS_BATCH_SIZE_COUNT, remainingBlocks);

                glBindBuffer(GL_ARRAY_BUFFER, blockPosInstVbo);
                glBufferSubData(GL_ARRAY_BUFFER, 0, batchSize*sizeof(glm::vec3), blockPositions.data()+renderedBlocks);

                glBindBuffer(GL_ARRAY_BUFFER, blockTypeInstVbo);
                glBufferSubData(GL_ARRAY_BUFFER, 0, batchSize*sizeof(float), blockTexIds.data()+renderedBlocks);

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
    glDeleteTextures(1, &blockTexArray);
    glDeleteBuffers(1, &blockVbo);
    glDeleteBuffers(1, &blockPosInstVbo);
    glDeleteVertexArrays(1, &blockVao);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
