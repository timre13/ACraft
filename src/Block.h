#pragma once

#include "Texture.h"
#include "ShaderProg.h"
#include "types.h"
#include <vector>
#include <string>
#include <array>

enum BlockType
{
    BLOCK_TYPE_AIR,
    BLOCK_TYPE_COBBLESTONE,
    BLOCK_TYPE_DIRT,
    BLOCK_TYPE_GRASS,
    BLOCK_TYPE_STONE,
    BLOCK_TYPE_BEDROCK,
    BLOCK_TYPE_DEEPSLATE,
    BLOCK_TYPE_COAL_ORE,
    BLOCK_TYPE_DEEPSLATE_COAL_ORE,
    BLOCK_TYPE__COUNT,
};

#define BLOCK_TEXTURE_DIR "../textures"
constexpr std::array<const char*, (int)BLOCK_TYPE__COUNT> blockTexturePaths = {
    "", // Air, skipped
    "cobblestone.png",
    "dirt.png",
    "grass.png",
    "stone.png",
    "bedrock.png",
    "deepslate.png",
    "coal_ore.png",
    "deepslate_coal_ore.png",
};

struct Block
{
    BlockType type{};
};

/*
 * Singleton class that handles block texture loading, VRAM buffer initialization and rendering.
 */
class BlockStuffHandler final
{
private:
    static uint s_texArray;
    static uint s_vao;
    static uint s_vbo;
    static uint s_posInstVbo;
    static uint s_typeInstVbo;
    static ShaderProg s_blockShaderProg;

    /*
     * Called by `get()` when it is called first time.
     */
    BlockStuffHandler();
    void loadBlockTextures();
    void setupBlockBuffers();

public:
    static inline BlockStuffHandler& get()
    {
        static BlockStuffHandler instance;
        return instance;
    }

    static void renderBlocks(std::vector<glm::vec3>& blockPositions, std::vector<int>& blockTexIds);

    ~BlockStuffHandler();
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

// Number of blocks renderd at once
#define BLOCK_POS_BATCH_SIZE_COUNT 1024*1024*10
