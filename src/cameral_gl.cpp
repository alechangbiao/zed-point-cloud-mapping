#include "camera_gl.h"

const sl::Translation CameraGL::ORIGINAL_FORWARD = sl::Translation(0, 0, 1);
const sl::Translation CameraGL::ORIGINAL_UP = sl::Translation(0, 1, 0);
const sl::Translation CameraGL::ORIGINAL_RIGHT = sl::Translation(1, 0, 0);

CameraGL::CameraGL(sl::Translation position, sl::Translation direction, sl::Translation vertical)
{
    this->position_ = position;
    setDirection(direction, vertical);

    offset_ = sl::Translation(0, 0, 0);
    view_.setIdentity();
    updateView();
    setProjection(80, 80, 100.f, 900000.f);
    updateVPMatrix();
}

CameraGL::~CameraGL() {}

void CameraGL::update()
{
    if (sl::Translation::dot(vertical_, up_) < 0)
        vertical_ = vertical_ * -1.f;
    updateView();
    updateVPMatrix();
}

void CameraGL::setProjection(float horizontalFOV, float verticalFOV, float znear, float zfar)
{
    horizontalFieldOfView_ = horizontalFOV;
    verticalFieldOfView_ = verticalFOV;
    znear_ = znear;
    zfar_ = zfar;

    float fov_y = verticalFOV * M_PI / 180.f;
    float fov_x = horizontalFOV * M_PI / 180.f;

    projection_.setIdentity();
    projection_(0, 0) = 1.0f / tanf(fov_x * 0.5f);
    projection_(1, 1) = 1.0f / tanf(fov_y * 0.5f);
    projection_(2, 2) = -(zfar + znear) / (zfar - znear);
    projection_(3, 2) = -1;
    projection_(2, 3) = -(2.f * zfar * znear) / (zfar - znear);
    projection_(3, 3) = 0;
}

const sl::Transform &CameraGL::getViewProjectionMatrix() const
{
    return vpMatrix_;
}

float CameraGL::getHorizontalFOV() const
{
    return horizontalFieldOfView_;
}

float CameraGL::getVerticalFOV() const
{
    return verticalFieldOfView_;
}

void CameraGL::setOffsetFromPosition(const sl::Translation &o)
{
    offset_ = o;
}

const sl::Translation &CameraGL::getOffsetFromPosition() const
{
    return offset_;
}

void CameraGL::setDirection(const sl::Translation &direction, const sl::Translation &vertical)
{
    sl::Translation dirNormalized = direction;
    dirNormalized.normalize();
    this->rotation_ = sl::Orientation(ORIGINAL_FORWARD, dirNormalized * -1.f);
    updateVectors();
    this->vertical_ = vertical;
    if (sl::Translation::dot(vertical_, up_) < 0)
        rotate(sl::Rotation(M_PI, ORIGINAL_FORWARD));
}

void CameraGL::translate(const sl::Translation &t)
{
    position_ = position_ + t;
}

void CameraGL::setPosition(const sl::Translation &p)
{
    position_ = p;
}

void CameraGL::rotate(const sl::Orientation &rot)
{
    rotation_ = rot * rotation_;
    updateVectors();
}

void CameraGL::rotate(const sl::Rotation &m)
{
    this->rotate(sl::Orientation(m));
}

void CameraGL::setRotation(const sl::Orientation &rot)
{
    rotation_ = rot;
    updateVectors();
}

void CameraGL::setRotation(const sl::Rotation &m)
{
    this->setRotation(sl::Orientation(m));
}

const sl::Translation &CameraGL::getPosition() const
{
    return position_;
}

const sl::Translation &CameraGL::getForward() const
{
    return forward_;
}

const sl::Translation &CameraGL::getRight() const
{
    return right_;
}

const sl::Translation &CameraGL::getUp() const
{
    return up_;
}

const sl::Translation &CameraGL::getVertical() const
{
    return vertical_;
}

float CameraGL::getZNear() const
{
    return znear_;
}

float CameraGL::getZFar() const
{
    return zfar_;
}

void CameraGL::updateVectors()
{
    forward_ = ORIGINAL_FORWARD * rotation_;
    up_ = ORIGINAL_UP * rotation_;
    right_ = sl::Translation(ORIGINAL_RIGHT * -1.f) * rotation_;
}

void CameraGL::updateView()
{
    // camera's model matrix in world space
    sl::Transform transformation(rotation_, (offset_ * rotation_) + position_);
    // get view matrix by transforms vertices from world space to camera space, or view space.
    view_ = sl::Transform::inverse(transformation);
}

void CameraGL::updateVPMatrix()
{
    vpMatrix_ = projection_ * view_;
}