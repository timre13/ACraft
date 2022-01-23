#pragma once

#include "types.h"
#include <string>

class Texture final
{
private:
    uint m_texId{};

public:
    Texture(const std::string& path);

    void bind();

    ~Texture();
};
