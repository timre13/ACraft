#pragma once

#include "Texture.h"
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
