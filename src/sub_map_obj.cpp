#include "sub_map_obj.h"

SubMapObj::SubMapObj()
{
    current_fpc_count_ = 0;
    vaoID_ = 0;
}

SubMapObj::~SubMapObj()
{
    current_fpc_count_ = 0;
    if (vaoID_)
    {
        glDeleteBuffers(2, vboID_);
        glDeleteVertexArrays(1, &vaoID_);
    }
}

void SubMapObj::update(sl::PointCloudChunk &chunk)
{
    if (vaoID_ == 0)
    {
        glGenVertexArrays(1, &vaoID_);
        glGenBuffers(2, vboID_);
    }

    glShadeModel(GL_SMOOTH);

    const auto nb_v = chunk.vertices.size();
    index.resize(nb_v);
    for (int c = 0; c < nb_v; c++)
        index[c] = c;

    glBindVertexArray(vaoID_);

    glBindBuffer(GL_ARRAY_BUFFER, vboID_[Shader::ATTRIB_VERTICES_POS]);
    glBufferData(GL_ARRAY_BUFFER, chunk.vertices.size() * sizeof(sl::float4), &chunk.vertices[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(Shader::ATTRIB_VERTICES_POS, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(Shader::ATTRIB_VERTICES_POS);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID_[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof(sl::uint1), &index[0], GL_DYNAMIC_DRAW);
    current_fpc_count_ = (int)index.size();

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SubMapObj::draw()
{
    if (current_fpc_count_ && vaoID_)
    {
        glBindVertexArray(vaoID_);
        glDrawElements(GL_POINTS, (GLsizei)current_fpc_count_, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}