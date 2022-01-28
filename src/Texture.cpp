#include "Texture.h"
#include "Logger.h"
#include "glstuff.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG // Get better error messages
#include "../deps/stb/stb_image.h"

Texture::Texture(const std::string& path)
{
    open(path);
}

void Texture::open(const std::string& path)
{
    Logger::log << "Loading texture: \"" << path << '"' << Logger::End;

    stbi_set_flip_vertically_on_load(1);

    int width{};
    int height{};
    int _{};
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &_, 4);

    if (data)
    {
        glGenTextures(1, &m_texId);
        glBindTexture(GL_TEXTURE_2D, m_texId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(data);

        Logger::dbg << "Successfully loaded texture (id=" << m_texId
            << ", width=" << width << ", height=" << height << ")" << Logger::End;
    }
    else
    {
        Logger::fatal << "Failed to load texture: " << stbi_failure_reason() << Logger::End;
    }
}

void Texture::bind()
{
    glBindTexture(GL_TEXTURE_2D, m_texId);
}

Texture::~Texture()
{
    if (m_texId)
    {
        glDeleteSamplers(1, &m_texId);
        Logger::dbg << "Texture " << m_texId << " deleted" << Logger::End;
    }
}
