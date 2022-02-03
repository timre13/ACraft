#pragma once

#include "types.h"
#include <string>

class Texture final
{
private:
    uint m_texId{};
    std::string m_path;

public:
    Texture() {}
    Texture(const std::string& path);

    void open(const std::string& path);

    void bind();

    inline bool isEmpty() const { return m_texId == 0; }

    ~Texture();
};
