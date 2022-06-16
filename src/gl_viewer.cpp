#include "gl_viewer.h"

GLViewer *currentInstance_ = nullptr;

void CloseFunc(void)
{
    if (currentInstance_)
        currentInstance_->exit();
}

void fillZED(int nb_tri, float *vertices, int *triangles, sl::float3 color, Simple3DObject *zed_camera)
{
    for (int p = 0; p < nb_tri * 3; p = p + 3)
    {
        int index = triangles[p] - 1;
        zed_camera->addPoint(sl::float3(vertices[index * 3], vertices[index * 3 + 1], vertices[index * 3 + 2]) * 1000, sl::float3(color.r, color.g, color.b));
        index = triangles[p + 1] - 1;
        zed_camera->addPoint(sl::float3(vertices[index * 3], vertices[index * 3 + 1], vertices[index * 3 + 2]) * 1000, sl::float3(color.r, color.g, color.b));
        index = triangles[p + 2] - 1;
        zed_camera->addPoint(sl::float3(vertices[index * 3], vertices[index * 3 + 1], vertices[index * 3 + 2]) * 1000, sl::float3(color.r, color.g, color.b));
    }
}

GLViewer::GLViewer() : available(false)
{
    currentInstance_ = this;
    mouseButton_[0] = mouseButton_[1] = mouseButton_[2] = false;
    clearInputs();
    previousMouseMotion_[0] = previousMouseMotion_[1] = 0;
}

GLViewer::~GLViewer() {}

void GLViewer::exit()
{
    if (currentInstance_)
        available = false;
}

bool GLViewer::isAvailable()
{
    if (available)
        glutMainLoopEvent();
    return available;
}

GLenum GLViewer::init(int argc, char **argv,
                      sl::CameraParameters param, sl::FusedPointCloud *ptr, sl::MODEL zed_model)
{

    glutInit(&argc, argv);
    int wnd_w = glutGet(GLUT_SCREEN_WIDTH);
    int wnd_h = glutGet(GLUT_SCREEN_HEIGHT) * 0.9;
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(wnd_w * 0.05, wnd_h * 0.05);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("ZED PointCloud Fusion");

    GLenum err = glewInit();
    if (GLEW_OK != err)
        return err;

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    p_fpc = ptr;

    // Compile and create the shader
    mainShader.it = Shader(VERTEX_SHADER, FRAGMENT_SHADER);
    mainShader.MVP_Mat = glGetUniformLocation(mainShader.it.getProgramId(), "u_mvpMatrix");

    pcf_shader.it = Shader(FPC_VERTEX_SHADER, FRAGMENT_SHADER);
    pcf_shader.MVP_Mat = glGetUniformLocation(pcf_shader.it.getProgramId(), "u_mvpMatrix");

    // Create the camera
    camera_ = CameraGL(sl::Translation(0, 0, 1000), sl::Translation(0, 0, -100));
    camera_.setOffsetFromPosition(sl::Translation(0, 0, 1500));

    // change background color
    bckgrnd_clr = sl::float3(37, 42, 44);
    bckgrnd_clr /= 255.f;

    zedPath.setDrawingType(GL_LINE_STRIP);
    zedModel.setDrawingType(GL_TRIANGLES);
    Model3D *model;
    switch (zed_model)
    {
    case sl::MODEL::ZED:
        model = new Model3D_ZED();
        break;
    case sl::MODEL::ZED2:
        model = new Model3D_ZED2();
        break;
    case sl::MODEL::ZED_M:
        model = new Model3D_ZED_M();
        break;
    }
    for (auto it : model->part)
        fillZED(it.nb_triangles, model->vertices, it.triangles, it.color, &zedModel);
    delete model;

    zedModel.pushToGPU();
    updateZEDposition = false;

    // Map glut function on this class methods
    glutDisplayFunc(GLViewer::drawCallback);
    glutMouseFunc(GLViewer::mouseButtonCallback);
    glutMotionFunc(GLViewer::mouseMotionCallback);
    glutReshapeFunc(GLViewer::reshapeCallback);
    glutKeyboardFunc(GLViewer::keyPressedCallback);
    glutKeyboardUpFunc(GLViewer::keyReleasedCallback);
    glutCloseFunc(CloseFunc);

    available = true;

    // ready to start
    chunks_pushed = true;

    return err;
}

void GLViewer::render()
{
    if (available)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(bckgrnd_clr.r, bckgrnd_clr.g, bckgrnd_clr.b, 1.f);
        update();
        draw();
        printText();
        glutSwapBuffers();
        glutPostRedisplay();
    }
}

void GLViewer::update()
{
    if (keyStates_['q'] == KEY_STATE::UP || keyStates_['Q'] == KEY_STATE::UP || keyStates_[27] == KEY_STATE::UP)
    {
        currentInstance_->exit();
        return;
    }

    if (keyStates_['f'] == KEY_STATE::UP || keyStates_['F'] == KEY_STATE::UP)
    {
        followCamera = !followCamera;
        if (followCamera)
            camera_.setOffsetFromPosition(sl::Translation(0, 0, 1500));
    }

    // Rotate camera with mouse
    if (!followCamera)
    {
        if (mouseButton_[MOUSE_BUTTON::LEFT])
        {
            camera_.rotate(sl::Rotation((float)mouseMotion_[1] * MOUSE_R_SENSITIVITY, camera_.getRight()));
            camera_.rotate(sl::Rotation((float)mouseMotion_[0] * MOUSE_R_SENSITIVITY, camera_.getVertical() * -1.f));
        }

        // Translate camera with mouse
        if (mouseButton_[MOUSE_BUTTON::RIGHT])
        {
            camera_.translate(camera_.getUp() * (float)mouseMotion_[1] * MOUSE_T_SENSITIVITY);
            camera_.translate(camera_.getRight() * (float)mouseMotion_[0] * MOUSE_T_SENSITIVITY);
        }
    }

    // Zoom in with mouse wheel
    if (mouseWheelPosition_ != 0)
    {
        sl::Translation cur_offset = camera_.getOffsetFromPosition();
        bool zoom_ = mouseWheelPosition_ > 0;
        sl::Translation new_offset = cur_offset * (zoom_ ? MOUSE_UZ_SENSITIVITY : MOUSE_DZ_SENSITIVITY);
        if (zoom_)
        {
            if (followCamera)
            {
                if ((new_offset.tz < 500.f))
                    new_offset.tz = 500.f;
            }
            else
            {
                if ((new_offset.tz < 50.f))
                    new_offset.tz = 50.f;
            }
        }
        else
        {
            if (followCamera)
            {
                if (new_offset.tz > 5000.f)
                    new_offset.tz = 5000.f;
            }
        }
        camera_.setOffsetFromPosition(new_offset);
    }

    // Update point cloud buffers
    camera_.update();
    clearInputs();
    mtx.lock();
    if (updateZEDposition)
    {
        sl::float3 clr(0.1f, 0.5f, 0.9f);
        for (auto it : vecPath)
            zedPath.addPoint(it, clr);
        zedPath.pushToGPU();
        vecPath.clear();
        updateZEDposition = false;
    }

    if (new_chunks)
    {
        const int nb_c = p_fpc->chunks.size();
        if (nb_c > sub_maps.size())
        {
            const float step = 500.f;
            size_t new_size = ((nb_c / step) + 1) * step;
            sub_maps.resize(new_size);
        }
        int c = 0;
        for (auto &it : sub_maps)
        {
            if ((c < nb_c) && p_fpc->chunks[c].has_been_updated)
                it.update(p_fpc->chunks[c]);
            c++;
        }

        new_chunks = false;
        chunks_pushed = true;
    }

    mtx.unlock();
}

void GLViewer::draw()
{
    const sl::Transform vpMatrix = camera_.getViewProjectionMatrix();

    glUseProgram(mainShader.it.getProgramId());
    glUniformMatrix4fv(mainShader.MVP_Mat, 1, GL_TRUE, vpMatrix.m);

    glLineWidth(1.f);
    zedPath.draw();

    glUniformMatrix4fv(
        mainShader.MVP_Mat, 1, GL_FALSE,
        (sl::Transform::transpose(zedModel.getModelMatrix()) * sl::Transform::transpose(vpMatrix)).m);

    zedModel.draw();
    glUseProgram(0);

    if (sub_maps.size())
    {
        glPointSize(2.f);
        glUseProgram(pcf_shader.it.getProgramId());
        glUniformMatrix4fv(pcf_shader.MVP_Mat, 1, GL_TRUE, vpMatrix.m);

        for (auto &it : sub_maps)
            it.draw();
        glUseProgram(0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void GLViewer::clearInputs()
{
    mouseMotion_[0] = mouseMotion_[1] = 0;
    mouseWheelPosition_ = 0;
    for (unsigned int i = 0; i < 256; ++i)
        if (keyStates_[i] != KEY_STATE::DOWN)
            keyStates_[i] = KEY_STATE::FREE;
}

void GLViewer::drawCallback()
{
    currentInstance_->render();
}

void printGL(float x, float y, const char *string)
{
    glRasterPos2f(x, y);
    int len = (int)strlen(string);
    for (int i = 0; i < len; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string[i]);
    }
}

void GLViewer::printText()
{
    if (available)
    {
        glColor3f(0.85f, 0.86f, 0.83f);
        printGL(-0.99f, 0.90f, "Press 'F' to un/follow the camera");

        std::string positional_tracking_state_str();
        // Show mapping state
        if ((tracking_state == sl::POSITIONAL_TRACKING_STATE::OK))
            glColor3f(0.25f, 0.99f, 0.25f);
        else
            glColor3f(0.99f, 0.25f, 0.25f);
        std::string state_str("POSITIONAL TRACKING STATE : ");
        state_str += sl::toString(tracking_state).c_str();
        printGL(-0.99f, 0.95f, state_str.c_str());
    }
}

void GLViewer::mouseButtonCallback(int button, int state, int x, int y)
{
    if (button < 5)
    {
        if (button < 3)
        {
            currentInstance_->mouseButton_[button] = state == GLUT_DOWN;
        }
        else
        {
            currentInstance_->mouseWheelPosition_ += button == MOUSE_BUTTON::WHEEL_UP ? 1 : -1;
        }
        currentInstance_->mouseCurrentPosition_[0] = x;
        currentInstance_->mouseCurrentPosition_[1] = y;
        currentInstance_->previousMouseMotion_[0] = x;
        currentInstance_->previousMouseMotion_[1] = y;
    }
}

void GLViewer::mouseMotionCallback(int x, int y)
{
    currentInstance_->mouseMotion_[0] = x - currentInstance_->previousMouseMotion_[0];
    currentInstance_->mouseMotion_[1] = y - currentInstance_->previousMouseMotion_[1];
    currentInstance_->previousMouseMotion_[0] = x;
    currentInstance_->previousMouseMotion_[1] = y;
    glutPostRedisplay();
}

void GLViewer::reshapeCallback(int width, int height)
{
    glViewport(0, 0, width, height);
    float hfov = (180.0f / M_PI) * (2.0f * atan(width / (2.0f * 500)));
    float vfov = (180.0f / M_PI) * (2.0f * atan(height / (2.0f * 500)));
    currentInstance_->camera_.setProjection(hfov, vfov, currentInstance_->camera_.getZNear(), currentInstance_->camera_.getZFar());
}

void GLViewer::keyPressedCallback(unsigned char c, int x, int y)
{
    currentInstance_->keyStates_[c] = KEY_STATE::DOWN;
    glutPostRedisplay();
}

void GLViewer::keyReleasedCallback(unsigned char c, int x, int y)
{
    currentInstance_->keyStates_[c] = KEY_STATE::UP;
}

void GLViewer::idle()
{
    glutPostRedisplay();
}

void GLViewer::updatePose(sl::Pose pose_, sl::POSITIONAL_TRACKING_STATE state)
{
    mtx.lock();
    pose = pose_;
    tracking_state = state;
    vecPath.push_back(pose.getTranslation());
    zedModel.setRT(pose.pose_data);
    updateZEDposition = true;
    mtx.unlock();
    if (followCamera)
    {
        camera_.setPosition(pose.getTranslation());
        sl::Rotation rot = pose.getRotationMatrix();
        camera_.setRotation(rot);
    }
}
