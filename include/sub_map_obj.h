#pragma once

#include <sl/Camera.hpp>
#include <GL/glew.h>

#include "shader.h"

class SubMapObj
{
    GLuint vaoID_;
    GLuint vboID_[2];
    int current_fc;

    std::vector<sl::uint1> index;

public:
    SubMapObj();
    ~SubMapObj();
    void update(sl::PointCloudChunk &chunks);
    void draw();
};