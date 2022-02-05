#pragma once

#include "types.h"
#include <string>

class Texture final
{
private:
    uint m_texId{};
    std::string m_path;

public:
    Texture() = default;
    Texture(const std::string& path);

    // Disable copying
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Implement moving
    Texture(Texture&& temp) noexcept;
    Texture& operator=(Texture&& temp) noexcept;

    void open(const std::string& path);

    void bind();

    inline bool isEmpty() const { return m_texId == 0; }

    ~Texture();
};
