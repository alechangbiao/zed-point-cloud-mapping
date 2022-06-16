#pragma once

#include <sl/Camera.hpp>
#include <GL/glew.h>

#include "shader.h"

class SubMapObj
{
    GLuint vaoID_;
    GLuint vboID_[2];

    /// represent the current count of fused point cloud chunk
    int current_fc;

    std::vector<sl::uint1> index;

public:
    SubMapObj();
    ~SubMapObj();

    /// Take the chunk of point cloud data, set up vao then push the data to GPU
    void update(sl::PointCloudChunk &chunks);
    void draw();
};