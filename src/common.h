#pragma once

#include "Logger.h"
#include <fstream>
#include <sstream>
#include <cstring>

inline std::string loadTextFile(const std::string& path)
{
    try
    {
        std::fstream file;
        // Throw on error
        file.exceptions(std::fstream::failbit|std::ifstream::badbit);
        file.open(path, std::ios::in);
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }
    catch (std::exception&)
    {
        Logger::fatal << "Failed to open file: \"" << path << "\": " << std::strerror(errno) << Logger::End;
        return ""; // Not reached
    }
}
