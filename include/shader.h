#pragma once

#include <GL/glew.h>

extern GLchar *VERTEX_SHADER;
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

struct ShaderData
{
    Shader it;
    GLuint MVP_Mat;
};