#pragma once

#include <GL/glew.h>

extern GLchar *VERTEX_SHADER;
/// PC - Point Cloud
/// F - Fused
extern GLchar *FPC_VERTEX_SHADER;
extern GLchar *FRAGMENT_SHADER;

class Shader
{
public:
    Shader() {}
    Shader(GLchar *vs, GLchar *fs);
    ~Shader();
    GLuint getProgramId();

    static const GLint ATTRIB_VERTICES_POS = 0;
    static const GLint ATTRIB_COLOR_POS = 1;

private:
    bool compile(GLuint &shaderId, GLenum type, GLchar *src);
    GLuint verterxId_;
    GLuint fragmentId_;
    GLuint programId_;
};

/// @brief A struct to represent ShaderData
///
/// @param it::Shader the shader object itself
/// @param MVP_Mat::GLuint the uniform id of the MVP Matrix
struct ShaderData
{
    Shader it;
    GLuint MVP_Mat;
};