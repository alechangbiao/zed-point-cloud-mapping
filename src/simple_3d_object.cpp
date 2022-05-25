#include "simple_3d_object.h"

Simple3DObject::Simple3DObject() : isStatic_(false)
{
    vaoID_ = 0;
    drawingType_ = GL_TRIANGLES;
    position_ = sl::float3(0, 0, 0);
    rotation_.setIdentity();
}

Simple3DObject::Simple3DObject(sl::Translation position, bool isStatic) : isStatic_(isStatic)
{
    vaoID_ = 0;
    drawingType_ = GL_TRIANGLES;
    position_ = position;
    rotation_.setIdentity();
}

Simple3DObject::~Simple3DObject()
{
    if (vaoID_ != 0)
    {
        glDeleteBuffers(3, vboID_);
        glDeleteVertexArrays(1, &vaoID_);
    }
}

void Simple3DObject::addPoint(sl::float3 position, sl::float3 color)
{
    addPoint(position.x, position.y, position.z, color.r, color.g, color.b);
}

void Simple3DObject::addPoint(float x, float y, float z, float r, float g, float b)
{
    vertices_.push_back(x);
    vertices_.push_back(y);
    vertices_.push_back(z);
    colors_.push_back(r);
    colors_.push_back(g);
    colors_.push_back(b);
    indices_.push_back((int)indices_.size());
}

void Simple3DObject::addLine(sl::float3 p1, sl::float3 p2, sl::float3 clr)
{
    vertices_.push_back(p1.x);
    vertices_.push_back(p1.y);
    vertices_.push_back(p1.z);

    vertices_.push_back(p2.x);
    vertices_.push_back(p2.y);
    vertices_.push_back(p2.z);

    colors_.push_back(clr.r);
    colors_.push_back(clr.g);
    colors_.push_back(clr.b);

    colors_.push_back(clr.r);
    colors_.push_back(clr.g);
    colors_.push_back(clr.b);

    indices_.push_back((int)indices_.size());
    indices_.push_back((int)indices_.size());
}

void Simple3DObject::pushToGPU()
{
    if (!isStatic_ || vaoID_ == 0)
    {
        if (vaoID_ == 0)
        {
            glGenVertexArrays(1, &vaoID_);
            glGenBuffers(3, vboID_);
        }
        glBindVertexArray(vaoID_);
        if (vertices_.size())
        {
            glBindBuffer(GL_ARRAY_BUFFER, vboID_[0]);
            glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(float), &vertices_[0], isStatic_ ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
            glVertexAttribPointer(Shader::ATTRIB_VERTICES_POS, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(Shader::ATTRIB_VERTICES_POS);
        }

        if (colors_.size())
        {
            glBindBuffer(GL_ARRAY_BUFFER, vboID_[1]);
            glBufferData(GL_ARRAY_BUFFER, colors_.size() * sizeof(float), &colors_[0], isStatic_ ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
            glVertexAttribPointer(Shader::ATTRIB_COLOR_POS, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(Shader::ATTRIB_COLOR_POS);
        }

        if (indices_.size())
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID_[2]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), &indices_[0], isStatic_ ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
        }

        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void Simple3DObject::clear()
{
    vertices_.clear();
    colors_.clear();
    indices_.clear();
}

void Simple3DObject::setDrawingType(GLenum type)
{
    drawingType_ = type;
}

void Simple3DObject::draw()
{
    if (indices_.size() && vaoID_)
    {
        glBindVertexArray(vaoID_);
        glDrawElements(drawingType_, (GLsizei)indices_.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void Simple3DObject::translate(const sl::Translation &t)
{
    position_ = position_ + t;
}

void Simple3DObject::setPosition(const sl::Translation &p)
{
    position_ = p;
}

void Simple3DObject::setRT(const sl::Transform &mRT)
{
    position_ = mRT.getTranslation();
    rotation_ = mRT.getOrientation();
}

void Simple3DObject::rotate(const sl::Orientation &rot)
{
    rotation_ = rot * rotation_;
}

void Simple3DObject::rotate(const sl::Rotation &m)
{
    this->rotate(sl::Orientation(m));
}

void Simple3DObject::setRotation(const sl::Orientation &rot)
{
    rotation_ = rot;
}

void Simple3DObject::setRotation(const sl::Rotation &m)
{
    this->setRotation(sl::Orientation(m));
}

const sl::Translation &Simple3DObject::getPosition() const
{
    return position_;
}

sl::Transform Simple3DObject::getModelMatrix() const
{
    sl::Transform tmp = sl::Transform::identity();
    tmp.setOrientation(rotation_);
    tmp.setTranslation(position_);
    return tmp;
}
