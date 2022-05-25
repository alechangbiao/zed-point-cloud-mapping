#pragma once

#include <sl/Camera.hpp>

class CameraGL
{
public:
    CameraGL() {}
    enum DIRECTION
    {
        UP,
        DOWN,
        LEFT,
        RIGHT,
        FORWARD,
        BACK
    };
    CameraGL(sl::Translation position, sl::Translation direction, sl::Translation vertical = sl::Translation(0, 1, 0)); // vertical = Eigen::Vector3f(0, 1, 0)
    ~CameraGL();

    void update();
    void setProjection(float horizontalFOV, float verticalFOV, float znear, float zfar);
    const sl::Transform &getViewProjectionMatrix() const;

    float getHorizontalFOV() const;
    float getVerticalFOV() const;

    // Set an offset between the eye of the camera and its position
    // Note: Useful to use the camera as a trackball camera with z>0 and x = 0, y = 0
    // Note: coordinates are in local space
    void setOffsetFromPosition(const sl::Translation &offset);
    const sl::Translation &getOffsetFromPosition() const;

    void setDirection(const sl::Translation &direction, const sl::Translation &vertical);
    void translate(const sl::Translation &t);
    void setPosition(const sl::Translation &p);
    void rotate(const sl::Orientation &rot);
    void rotate(const sl::Rotation &m);
    void setRotation(const sl::Orientation &rot);
    void setRotation(const sl::Rotation &m);

    const sl::Translation &getPosition() const;
    const sl::Translation &getForward() const;
    const sl::Translation &getRight() const;
    const sl::Translation &getUp() const;
    const sl::Translation &getVertical() const;
    float getZNear() const;
    float getZFar() const;

    static const sl::Translation ORIGINAL_FORWARD;
    static const sl::Translation ORIGINAL_UP;
    static const sl::Translation ORIGINAL_RIGHT;

    sl::Transform projection_;

private:
    void updateVectors();
    void updateView();
    void updateVPMatrix();

    sl::Translation offset_;
    sl::Translation position_;
    sl::Translation forward_;
    sl::Translation up_;
    sl::Translation right_;
    sl::Translation vertical_;

    sl::Orientation rotation_;

    sl::Transform view_;
    sl::Transform vpMatrix_;
    float horizontalFieldOfView_;
    float verticalFieldOfView_;
    float znear_;
    float zfar_;
};