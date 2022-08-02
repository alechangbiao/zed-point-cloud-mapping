#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <list>

#include "simple_3d_object.h"
#include "camera_gl.h"
#include "sub_map_obj.h"
#include "shader.h"

#include "zed_model.h"

#ifndef M_PI
#define M_PI 3.141592653f
#endif

const float MOUSE_R_SENSITIVITY = 0.025f;
const float MOUSE_UZ_SENSITIVITY = 0.75f;
const float MOUSE_DZ_SENSITIVITY = 1.25f;
const float MOUSE_T_SENSITIVITY = 80.f;
const float KEY_T_SENSITIVITY = 0.1f;

/// This class manages input events, window and Opengl rendering pipeline
class GLViewer
{
public:
    GLViewer();
    ~GLViewer();
    bool isAvailable();

    GLenum init(int argc, char **argv, sl::CameraParameters param,
                sl::FusedPointCloud *ptr, sl::MODEL zed_model);
    void updatePose(sl::Pose pose_, sl::POSITIONAL_TRACKING_STATE tracking_state);

    /// Set GLViewer::new_chunks (private) to true
    /// Set GLViewer::chunks_pushed (private) to false
    void updateChunks()
    {
        new_chunks = true;
        chunks_pushed = false;
    }

    bool chunksUpdated()
    {
        return chunks_pushed;
    }

    void exit();

private:
    // Rendering loop method called each frame by glutDisplayFunc
    void render();

    /// Update everything before rendering
    ///
    /// which includes:
    /// - camera's projection & view matrix
    /// - newly produced verticies of camera path and point cloud
    void update();

    // Once everything is updated, every renderable objects must be drawn in this method
    void draw();
    // Clear and refresh inputs' data
    void clearInputs();

    void printText();

    // Glut functions callbacks
    static void drawCallback();
    static void mouseButtonCallback(int button, int state, int x, int y);
    static void mouseMotionCallback(int x, int y);
    static void reshapeCallback(int width, int height);
    static void keyPressedCallback(unsigned char c, int x, int y);
    static void keyReleasedCallback(unsigned char c, int x, int y);
    static void idle();

    bool available;

    enum MOUSE_BUTTON
    {
        LEFT = 0,
        MIDDLE = 1,
        RIGHT = 2,
        WHEEL_UP = 3,
        WHEEL_DOWN = 4
    };

    enum KEY_STATE
    {
        UP = 'u',
        DOWN = 'd',
        FREE = 'f'
    };

    Simple3DObject zedModel_;
    Simple3DObject zedPath_;
    std::vector<sl::float3> vecPath;

    std::mutex mtx;
    bool updateZEDposition = false;
    ;

    bool mouseButton_[3];
    int mouseWheelPosition_;
    int mouseCurrentPosition_[2];
    int mouseMotion_[2];
    int previousMouseMotion_[2];
    KEY_STATE keyStates_[256];
    sl::float3 bckgrnd_clr;
    sl::Pose pose_;

    sl::POSITIONAL_TRACKING_STATE tracking_state;

    bool followCamera = true;
    bool new_chunks = false;
    bool chunks_pushed = false;

    CameraGL camera_;
    ShaderData mainShader;
    ShaderData pcf_shader;

    sl::FusedPointCloud *p_fpc;
    std::list<SubMapObj> sub_maps; // Opengl mesh container
};