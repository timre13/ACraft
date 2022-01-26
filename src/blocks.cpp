#include "blocks.h"

std::array<Texture, BLOCK_TYPE__COUNT> blockTextures;

void loadBlockTextures()
{
    for (size_t i{}; i < (size_t)BLOCK_TYPE__COUNT; ++i)
    {
        blockTextures[i].open(std::string(BLOCK_TEXTURE_DIR) + "/" + blockTexturePaths[i]);
    }
}
