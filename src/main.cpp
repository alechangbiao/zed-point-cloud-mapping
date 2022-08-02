///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2021, STEREOLABS.
//
// All rights reserved.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

/**********************************************************************************
 ** This sample demonstrates how to capture a live 3D reconstruction of a scene  **
 ** as a fused point cloud and display the result in an OpenGL window.           **
 **********************************************************************************/

// ZED includes
#include <sl/Camera.hpp>

// Sample includes
#include "gl_viewer.h"

#include "utils.h"

#include <opencv2/opencv.hpp>

int main(int argc, char **argv)
{
    sl::Camera zed;
    // Set configuration parameters for the ZED
    sl::InitParameters init_parameters;
    init_parameters.depth_mode = sl::DEPTH_MODE::ULTRA;
    init_parameters.coordinate_system = sl::COORDINATE_SYSTEM::RIGHT_HANDED_Y_UP; // OpenGL's coordinate system is right_handed
    parse_args(argc, argv, init_parameters);

    // Open the camera
    auto returned_state = zed.open(init_parameters);

    if (returned_state != sl::ERROR_CODE::SUCCESS)
    { // Quit if an error occurred
        print("Open Camera", returned_state, "\nExit program.");
        zed.close();
        return EXIT_FAILURE;
    }

    auto camera_infos = zed.getCameraInformation();

    // Point cloud viewer
    GLViewer viewer;

    // Initialize point cloud viewer
    sl::FusedPointCloud map;
    GLenum errgl = viewer.init(argc, argv,
                               camera_infos.camera_configuration.calibration_parameters.left_cam,
                               &map, camera_infos.camera_model);
    if (errgl != GLEW_OK)
        print("Error OpenGL: " + std::string((char *)glewGetErrorString(errgl)));

    // Setup and start positional tracking
    sl::Pose pose;
    sl::POSITIONAL_TRACKING_STATE tracking_state = sl::POSITIONAL_TRACKING_STATE::OFF;
    sl::PositionalTrackingParameters positional_tracking_parameters;
    positional_tracking_parameters.enable_area_memory = false;
    returned_state = zed.enablePositionalTracking(positional_tracking_parameters);
    if (returned_state != sl::ERROR_CODE::SUCCESS)
    {
        print("Enabling positional tracking failed: ", returned_state);
        zed.close();
        return EXIT_FAILURE;
    }

    // Set spatial mapping parameters
    sl::SpatialMappingParameters spatial_mapping_parameters;
    // Request a Point Cloud
    spatial_mapping_parameters.map_type = sl::SpatialMappingParameters::SPATIAL_MAP_TYPE::FUSED_POINT_CLOUD;
    // Set mapping range, it will set the resolution accordingly (a higher range, a lower resolution)
    spatial_mapping_parameters.set(sl::SpatialMappingParameters::MAPPING_RANGE::LONG);
    // Request partial updates only (only the lastest updated chunks need to be re-draw)
    spatial_mapping_parameters.use_chunk_only = true;
    // Start the spatial mapping
    zed.enableSpatialMapping(spatial_mapping_parameters);

    // Timestamp of the last fused point cloud requested
    std::chrono::high_resolution_clock::time_point ts_last;

    // Setup runtime parameters
    sl::RuntimeParameters runtime_parameters;
    // Use low depth confidence avoid introducing noise in the constructed model
    runtime_parameters.confidence_threshold = 50;

    auto resolution = camera_infos.camera_configuration.resolution;

    // Define display resolution and check that it fit at least the image resolution
    sl::Resolution display_resolution(std::min((int)resolution.width, 720),
                                      std::min((int)resolution.height, 404));

    // Create a Mat to contain the left image and its opencv ref
    sl::Mat image_zed(display_resolution, sl::MAT_TYPE::U8_C4);
    cv::Mat image_zed_ocv(image_zed.getHeight(), image_zed.getWidth(), CV_8UC4, image_zed.getPtr<sl::uchar1>(sl::MEM::CPU));

    // Start the main loop
    while (viewer.isAvailable())
    {
        // Grab a new image
        if (zed.grab(runtime_parameters) == sl::ERROR_CODE::SUCCESS)
        {
            // Retrieve the left image
            zed.retrieveImage(image_zed, sl::VIEW::LEFT, sl::MEM::CPU, display_resolution);
            // Retrieve the camera pose data
            tracking_state = zed.getPosition(pose);
            viewer.updatePose(pose, tracking_state);

            if (tracking_state == sl::POSITIONAL_TRACKING_STATE::OK)
            {
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - ts_last).count();

                // Ask for a fused point cloud update if 500ms have elapsed since last request
                if ((duration > 30) && viewer.chunksUpdated())
                {
                    // Ask for a point cloud refresh
                    zed.requestSpatialMapAsync();
                    ts_last = std::chrono::high_resolution_clock::now();
                }

                // If the point cloud is ready to be retrieved
                if (zed.getSpatialMapRequestStatusAsync() == sl::ERROR_CODE::SUCCESS)
                {
                    zed.retrieveSpatialMapAsync(map);
                    // std::cout << "Chunk Size: " << map.chunks.size() << std::endl;
                    viewer.updateChunks();
                }
            }
            cv::imshow("ZED View", image_zed_ocv);
            cv::waitKey(15);
        }
    }

    // Save generated point cloud
    // map.save("MyFusedPointCloud");

    // Free allocated memory before closing the camera
    image_zed.free();
    // Close the ZED
    zed.close();

    return 0;
}
