#pragma once

#include "types.h"
#include <string>

class ShaderProg
{
private:
    uint m_progId{};

    static uint s_boundProgId;

public:
    ShaderProg(const std::string& vertPath, const std::string& fragPath);

    void bind();

    ~ShaderProg();
};
