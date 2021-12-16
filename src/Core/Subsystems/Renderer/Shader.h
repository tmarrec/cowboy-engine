#pragma once

#include <glad/gl.h>

#include "../../utils.h"

class Shader
{
 public:
    Shader(const std::string& vertexFilePath, const std::string& fragmentFilePath);
    void use() const;

 private:
    void checkCompilation(const GLuint shader, const GLenum type) const;

    GLuint _id;
};
