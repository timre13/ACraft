#pragma once

#include "glstuff.h"
#include "types.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class ShaderProg
{
private:
    uint m_progId{};

    static uint s_boundProgId;

    int getUniformLocation(const char* name) const;

public:
    ShaderProg(const std::string& vertPath, const std::string& fragPath);

    void bind();

    void setUniform(const char* name, const glm::mat4& x);

    ~ShaderProg();
};
