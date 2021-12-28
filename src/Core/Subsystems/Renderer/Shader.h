#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../../utils.h"

struct MVP
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

class Shader
{
 public:
    Shader(const std::string& vertexFilePath, const std::string& fragmentFilePath);
    void setMat4f(const std::string& name, const glm::mat4& mat) const;
    void set3f(const std::string& name, const glm::vec3& v) const;
    void set1f(const std::string& name, const float f) const;
    void set1i(const std::string& name, const int i) const;
    void use() const;

 private:
    void checkCompilation(const GLuint shader, const GLenum type) const;

    GLuint _id;
};
