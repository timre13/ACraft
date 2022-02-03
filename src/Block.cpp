#include "Block.h"
#include "glstuff.h"
#include "../deps/stb/stb_image.h"
#include "Camera.h"
#include "Logger.h"
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>

extern Camera g_camera;

uint BlockStuffHandler::s_texArray = 0;
uint BlockStuffHandler::s_vao = 0;
uint BlockStuffHandler::s_vbo = 0;
uint BlockStuffHandler::s_posInstVbo = 0;
uint BlockStuffHandler::s_typeInstVbo = 0;
ShaderProg BlockStuffHandler::s_blockShaderProg = ShaderProg();

BlockStuffHandler::BlockStuffHandler()
{
    Logger::log << "Setting up block stuff" << Logger::End;

    s_blockShaderProg.open(
            "../src/shaders/block_inst.vert.glsl",
            "../src/shaders/block_inst.frag.glsl");
    loadBlockTextures();
    setupBlockBuffers();

    Logger::log << "Finished setting up block stuff" << Logger::End;
}

void BlockStuffHandler::loadBlockTextures()
{
    Logger::dbg << "Loading block textures" << Logger::End;

    int maxArrayTextureLayers{};
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxArrayTextureLayers);
    Logger::dbg << "GL_MAX_ARRAY_TEXTURE_LAYERS = " << maxArrayTextureLayers << Logger::End;
    // Check if we have space to store all the textures in a texture array
    assert(BLOCK_TYPE__COUNT <= maxArrayTextureLayers);

    glGenTextures(1, &s_texArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, s_texArray);
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

    Logger::dbg << "Finished loading block textures" << Logger::End;
}

void BlockStuffHandler::setupBlockBuffers()
{
    Logger::dbg << "Setting up block VRAM buffers" << Logger::End;

    glGenVertexArrays(1, &s_vao);
    glBindVertexArray(s_vao);

    {
        glGenBuffers(1, &s_vbo);

        glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
        glBufferData(GL_ARRAY_BUFFER, BLOCK_VERT_DATA_LEN*sizeof(float), &blockVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(VERT_ATTRIB_INDEX_MESH_COORDS, 3, GL_FLOAT, GL_FALSE, VALS_PER_VERT*sizeof(float), (void*)(0));
        glEnableVertexAttribArray(VERT_ATTRIB_INDEX_MESH_COORDS);

        glVertexAttribPointer(VERT_ATTRIB_INDEX_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, VALS_PER_VERT*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(VERT_ATTRIB_INDEX_TEX_COORDS);

        //----------------------------------------------------------------------

        glGenBuffers(1, &s_posInstVbo);

        glBindBuffer(GL_ARRAY_BUFFER, s_posInstVbo);
        glBufferData(GL_ARRAY_BUFFER, BLOCK_POS_BATCH_SIZE_COUNT*sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(VERT_ATTRIB_INDEX_INST_POS, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(VERT_ATTRIB_INDEX_INST_POS);
        glVertexAttribDivisor(VERT_ATTRIB_INDEX_INST_POS, 1); // Instanced attribute


        glGenBuffers(1, &s_typeInstVbo);

        glBindBuffer(GL_ARRAY_BUFFER, s_typeInstVbo);
        glBufferData(GL_ARRAY_BUFFER, BLOCK_POS_BATCH_SIZE_COUNT*sizeof(float), nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(VERT_ATTRIB_INDEX_INST_TYPE, 1, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(VERT_ATTRIB_INDEX_INST_TYPE);
        glVertexAttribDivisor(VERT_ATTRIB_INDEX_INST_TYPE, 1); // Instanced attribute
    }
    glBindVertexArray(0);

    Logger::dbg << "Finished setting up block VRAM buffers" << Logger::End;
}

void BlockStuffHandler::renderBlocks(std::vector<glm::vec3>& blockPositions, std::vector<int>& blockTexIds)
{
    glBindVertexArray(s_vao);
    s_blockShaderProg.bind();
    g_camera.updateShaderUniforms(s_blockShaderProg);

    int renderedBlocks{};
    int remainingBlocks = blockPositions.size();
    while (remainingBlocks > 0)
    {
        const int batchSize = std::min(BLOCK_POS_BATCH_SIZE_COUNT, remainingBlocks);

        glBindBuffer(GL_ARRAY_BUFFER, s_posInstVbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, batchSize*sizeof(glm::vec3), blockPositions.data()+renderedBlocks);

        glBindBuffer(GL_ARRAY_BUFFER, s_typeInstVbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, batchSize*sizeof(float), blockTexIds.data()+renderedBlocks);

        glDrawArraysInstanced(GL_TRIANGLES, 0, BLOCK_VERT_COUNT, batchSize);
        remainingBlocks -= BLOCK_POS_BATCH_SIZE_COUNT;
        renderedBlocks += BLOCK_POS_BATCH_SIZE_COUNT;
    }
}

BlockStuffHandler::~BlockStuffHandler()
{
    glDeleteTextures(1, &s_texArray);
    glDeleteBuffers(1, &s_vbo);
    glDeleteBuffers(1, &s_posInstVbo);
    glDeleteBuffers(1, &s_typeInstVbo);
    glDeleteVertexArrays(1, &s_vao);
    Logger::dbg << "Cleaned up block stuff" << Logger::End;
}
