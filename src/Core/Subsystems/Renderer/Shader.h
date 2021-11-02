#pragma once

#include <fstream>
#include <shaderc/shaderc.hpp>

#include "../../utils.h"

enum ShaderType
{
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_FRAGMENT
};

class Shader
{
 public:
    Shader(const std::string& filename, ShaderType type) :
        _filename{filename},
        _type{type}
    {};
    void compile();
    const std::vector<std::uint32_t>& code() const;

 private:
    std::string _filename;
    ShaderType _type;
    bool _lastCompilationOk = false;
    std::vector<std::uint32_t> _shaderCode = {};
};
