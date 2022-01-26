#pragma once

#include "Texture.h"
#include <string>
#include <array>

enum BlockType
{
    BLOCK_TYPE_COBBLESTONE,
    BLOCK_TYPE_DIRT,
    BLOCK_TYPE_GRASS,
    BLOCK_TYPE__COUNT,
};

#define BLOCK_TEXTURE_DIR "../textures"
constexpr std::array<const char*, (int)BLOCK_TYPE__COUNT> blockTexturePaths = {
    "cobblestone.png",
    "dirt.png",
    "grass.png",
};
extern std::array<Texture, BLOCK_TYPE__COUNT> blockTextures;

void loadBlockTextures();
