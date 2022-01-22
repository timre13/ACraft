#include "ShaderProg.h"
#include "common.h"
#include "glstuff.h"

uint ShaderProg::s_boundProgId = -1;

static uint setupShader(const std::string& path, bool isVert)
{
    const std::string shaderStr = loadTextFile(path);
    const char* shaderCStrP = shaderStr.c_str();

    uint shaderId = glCreateShader(isVert ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
    glShaderSource(shaderId, 1, &shaderCStrP, nullptr);
    glCompileShader(shaderId);
    int compStat{};
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compStat);

    if (compStat == GL_FALSE)
    {
        int infoLogLen{};
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLen);
        char* infoLog = new char[infoLogLen]{};
        glGetShaderInfoLog(shaderId, infoLogLen, nullptr, infoLog);
        Logger::fatal << "Failed to compile " << (isVert ? "vertex" : "fragment") << " shader: " << infoLog << Logger::End;
        delete[] infoLog; // Actually, never reached :(
        return -1;
    }
    else
    {
        Logger::dbg << "Successfully compiled " << (isVert ? "vertex" : "fragment") << " shader" << Logger::End;
        return shaderId;
    }

}

static uint setupProgram(uint vertId, uint fragId)
{
    uint progId = glCreateProgram();
    glAttachShader(progId, vertId);
    glAttachShader(progId, fragId);
    glLinkProgram(progId);

    int linkStat{};
    glGetProgramiv(progId, GL_LINK_STATUS, &linkStat);
    if (linkStat == GL_FALSE)
    {
        int infoLogLen{};
        glGetProgramiv(progId, GL_INFO_LOG_LENGTH, &infoLogLen);
        char* infoLog = new char[infoLogLen]{};
        glGetProgramInfoLog(progId, infoLogLen, nullptr, infoLog);
        Logger::fatal << "Failed to link shader program: " << infoLog << Logger::End;
        delete[] infoLog; // Actually, never reached :(
        return -1;
    }
    else
    {
        Logger::dbg << "Successfully linked shader program" << Logger::End;
        return progId;
    }
}

ShaderProg::ShaderProg(const std::string& vertPath, const std::string& fragPath)
{
    Logger::log << "Loading vertex shader: " << vertPath << Logger::End;
    uint vertId = setupShader(vertPath, true);
    Logger::log << "Loaded vertex shader (id=" << vertId << ")" << Logger::End;

    Logger::log << "Loading fragment shader: " << fragPath << Logger::End;
    uint fragId = setupShader(fragPath, false);
    Logger::log << "Loaded fragment shader (id=" << fragId << ")" << Logger::End;

    Logger::log << "Linking shader program" << Logger::End;
    m_progId = setupProgram(vertId, fragId);
    glDeleteShader(vertId);
    glDeleteShader(fragId);
    Logger::log << "Set up shader program" << Logger::End;
}

void ShaderProg::bind()
{
    // If the currently bound program is not this one, bind it
    if (s_boundProgId != m_progId)
    {
        glUseProgram(m_progId);
        s_boundProgId = m_progId;
    }
}

ShaderProg::~ShaderProg()
{
    glDeleteProgram(m_progId);
    Logger::dbg << "Shader program " << m_progId << " deleted" << Logger::End;
}
