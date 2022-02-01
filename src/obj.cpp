#include "obj.h"
#include "Logger.h"
#include "common.h"

#define MODEL_FILE_PARSER_VERBOSE 0

std::vector<float> loadObjFile(const std::string& path)
{
    Logger::log << "Loading model: " << path << Logger::End;

    const std::string fileContents = loadTextFile(path);

    std::vector<float> verticesTmp;
    std::vector<float> uvCoordsTmp;
    //std::vector<float> normalsTmp;
    std::vector<float> vertices;
    std::vector<float> uvCoords;
    //std::vector<float> normals;

    for (size_t i{}; i < fileContents.size();)
    {
        std::string token;

        // Skip comments
        if (fileContents[i] == '#')
        {
            while (i < fileContents.size() && fileContents[i] != '\n')
                ++i;
        }

        auto getToken{
            [&](){
                token.clear();

                // Get token
                while (i < fileContents.size() && !std::isspace((unsigned char)fileContents[i]))
                    token += fileContents[i++];
    
                // Skip whitespace
                while (i < fileContents.size() && (fileContents[i] == ' ' || fileContents[i] == '\t'))
                    ++i;
            }
        };

        auto tokenToFloat{
            [&](){
                try
                {
                    return std::stof(token.c_str(), nullptr);
                }
                catch (...)
                {
                    return 0.0f;
                }
            }
        };

        getToken();

        if (token.compare("v") == 0)
        {
            getToken();
            float vertexX = tokenToFloat();
            getToken();
            float vertexY = tokenToFloat();
            getToken();
            float vertexZ = tokenToFloat();

#if MODEL_FILE_PARSER_VERBOSE
            Logger::dbg << "Vertex(" << vertexX << ", " << vertexY << ", " << vertexZ << ")" << Logger::End;
#endif

            verticesTmp.push_back(vertexX);
            verticesTmp.push_back(vertexY);
            verticesTmp.push_back(vertexZ);
        }
        else if (token.compare("vt") == 0)
        {
            getToken();
            float textureX = tokenToFloat();
            getToken();
            float textureY = tokenToFloat();

#if MODEL_FILE_PARSER_VERBOSE
            Logger::dbg << "TexCoord(" << textureX << ", " << textureY << ")" << Logger::End;
#endif

            uvCoordsTmp.push_back(textureX);
            uvCoordsTmp.push_back(textureY);
        }
#if 0
        else if (token.compare("vn") == 0)
        {
            getToken();
            float normalX = tokenToFloat();
            getToken();
            float normalY = tokenToFloat();
            getToken();
            float normalZ = tokenToFloat();

#if MODEL_FILE_PARSER_VERBOSE
            Logger::dbg << "Normal(" << normalX << ", " << normalY << ", " << normalZ << ")" << Logger::End;
#endif

            normalsTmp.push_back(normalX);
            normalsTmp.push_back(normalY);
            normalsTmp.push_back(normalZ);
        }
#endif
        else if (token.compare("f") == 0)
        {
            for (int i{}; i < 3; ++i)
            {
                getToken();
                std::string originalToken = token;
                size_t vertexI, uvCoordI, normalI;
                size_t numOfProcessed;

                vertexI = std::stoul(token.c_str(), &numOfProcessed);

                try
                {
                    token = token.substr(numOfProcessed+1);
                    uvCoordI = std::stoul(token.c_str(), &numOfProcessed);
                }
                catch (...)
                {
                    Logger::err << "Invalid face specifier (UV coordinates missing? Make sure to export model with UV coordinates included)" << Logger::End;
                    return {};
                }

                try
                {
                    token = token.substr(numOfProcessed+1);
                    normalI = std::stoul(token.c_str(), &numOfProcessed);
                }
                catch (...)
                {
                    Logger::err << "Invalid face specifier (Normals missing? Make sure to export model with normals included)" << Logger::End;
                    return {};
                }

                if (vertexI  < 1 || vertexI  > verticesTmp.size()
                 || uvCoordI < 1 || uvCoordI > uvCoordsTmp.size()
                /* || normalI  < 1 || normalI  > normalsTmp.size()*/)
                {
                    Logger::err << "Invalid face vertex: " << originalToken
                        << ": FaceVertex(" << vertexI << ", " << uvCoordI << ", " << normalI << ")"  << Logger::End;
                    Logger::log << verticesTmp.size() << ", " << uvCoordsTmp.size()/* << ", " << normalsTmp.size()*/ << Logger::End;
                    return {};
                }

#if MODEL_FILE_PARSER_VERBOSE
                Logger::dbg << "FaceVertex(" << vertexI << ", " << uvCoordI << ", " << normalI << ")" << Logger::End;
#endif

                vertices.push_back(verticesTmp[(vertexI-1)*3+0]);
                vertices.push_back(verticesTmp[(vertexI-1)*3+1]);
                vertices.push_back(verticesTmp[(vertexI-1)*3+2]);
                uvCoords.push_back(uvCoordsTmp[(uvCoordI-1)*2+0]);
                uvCoords.push_back(uvCoordsTmp[(uvCoordI-1)*2+1]);
                //normals.push_back(normalsTmp[(normalI-1)*3+0]);
                //normals.push_back(normalsTmp[(normalI-1)*3+1]);
                //normals.push_back(normalsTmp[(normalI-1)*3+2]);
            }
            getToken();
            if (token.size())
            {
                Logger::err << "Face element with 3+ values. Re-export model with \"Triangulate faces\" option" << Logger::End;
                return {};
            }
        }
        else
        {
#if MODEL_FILE_PARSER_VERBOSE
            Logger::dbg << "Skipping unsupported keyword: " << token << Logger::End;
#endif
        }

        // Skip the remaining characters in the line
        while (i < fileContents.size() && fileContents[i] != '\n')
            ++i;
        ++i;
    }

    Logger::dbg << "Parsed a model with "
        << vertices.size()/3 << " vertices and "
        << uvCoords.size()/2 << " UV coordinates"
        //<< normals.size()/3 << " normals"
        << Logger::End;

#if MODEL_FILE_PARSER_VERBOSE
    Logger::dbg << "Vertices: ";
    for (float v : vertices)
        Logger::dbg << v << ", ";
    Logger::dbg << Logger::End;

    Logger::dbg << "UVs: ";
    for (float v : uvCoords)
        Logger::dbg << v << ", ";
    Logger::dbg << Logger::End;

    //Logger::dbg << "Normals: ";
    //for (float v : normals)
    //    Logger::dbg << v << ", ";
    //Logger::dbg << Logger::End;
#endif

    std::vector<float> output;
    for (size_t i{}; i < vertices.size()/3; ++i)
    {
        /*
         * Vertex data layout:
         *  * vertex (3 values)
         *  * UV coordinates (2 values)
         */

        output.push_back(vertices[i*3+0]);
        output.push_back(vertices[i*3+1]);
        output.push_back(vertices[i*3+2]);
        output.push_back(uvCoords[i*2+0]);
        output.push_back(uvCoords[i*2+1]);
        //m_vboData[arrayI+5] = normals[i+0];
        //m_vboData[arrayI+6] = normals[i+1];
        //m_vboData[arrayI+7] = normals[i+2];
    }
    return output;
}
