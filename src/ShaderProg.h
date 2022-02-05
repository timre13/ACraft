#pragma once

#include "glstuff.h"
#include "types.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class ShaderProg final
{
private:
    uint m_progId{};
    std::string m_vertPath;
    std::string m_fragPath;

    static uint s_boundProgId;

    int getUniformLocation(const char* name) const;

public:
    ShaderProg() = default;
    ShaderProg(const std::string& vertPath, const std::string& fragPath);
    // Disable copying
    ShaderProg(const ShaderProg&) = delete;
    ShaderProg& operator=(const ShaderProg&) = delete;

    // Implement moving
    ShaderProg(ShaderProg&& temp) noexcept;
    ShaderProg& operator=(ShaderProg&& temp) noexcept;

    void open(const std::string& vertPath, const std::string& fragPath);

    void bind();

    void setUniform(const char* name, const glm::mat4& x);

    ~ShaderProg();
};
