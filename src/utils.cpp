#include "utils.h"

// using namespace std;
// using namespace sl;

void parse_args(int argc, char **argv, sl::InitParameters &param)
{
    if (argc > 1 && std::string(argv[1]).find(".svo") != std::string::npos)
    {
        // SVO input mode
        param.input.setFromSVOFile(argv[1]);
        param.svo_real_time_mode = true;

        std::cout << "[Sample] Using SVO File input: " << argv[1] << std::endl;
    }
    else if (argc > 1 && std::string(argv[1]).find(".svo") == std::string::npos)
    {
        std::string arg = std::string(argv[1]);
        unsigned int a, b, c, d, port;
        if (sscanf(arg.c_str(), "%u.%u.%u.%u:%d", &a, &b, &c, &d, &port) == 5)
        {
            // Stream input mode - IP + port
            std::string ip_adress = std::to_string(a) + "." + std::to_string(b) + "." + std::to_string(c) + "." + std::to_string(d);
            param.input.setFromStream(sl::String(ip_adress.c_str()), port);
            std::cout << "[Sample] Using Stream input, IP : " << ip_adress << ", port : " << port << std::endl;
        }
        else if (sscanf(arg.c_str(), "%u.%u.%u.%u", &a, &b, &c, &d) == 4)
        {
            // Stream input mode - IP only
            param.input.setFromStream(sl::String(argv[1]));
            std::cout << "[Sample] Using Stream input, IP : " << argv[1] << std::endl;
        }
        else if (arg.find("HD2K") != std::string::npos)
        {
            param.camera_resolution = sl::RESOLUTION::HD2K;
            std::cout << "[Sample] Using Camera in resolution HD2K" << std::endl;
        }
        else if (arg.find("HD1080") != std::string::npos)
        {
            param.camera_resolution = sl::RESOLUTION::HD1080;
            std::cout << "[Sample] Using Camera in resolution HD1080" << std::endl;
        }
        else if (arg.find("HD720") != std::string::npos)
        {
            param.camera_resolution = sl::RESOLUTION::HD720;
            std::cout << "[Sample] Using Camera in resolution HD720" << std::endl;
        }
        else if (arg.find("VGA") != std::string::npos)
        {
            param.camera_resolution = sl::RESOLUTION::VGA;
            std::cout << "[Sample] Using Camera in resolution VGA" << std::endl;
        }
    }
    else
    {
        // Default
    }
}

void print(std::string msg_prefix, sl::ERROR_CODE err_code, std::string msg_suffix)
{
    std::cout << "[Sample]";
    if (err_code != sl::ERROR_CODE::SUCCESS)
        std::cout << "[Error] ";
    else
        std::cout << " ";
    std::cout << msg_prefix << " ";
    if (err_code != sl::ERROR_CODE::SUCCESS)
    {
        std::cout << " | " << toString(err_code) << " : ";
        std::cout << toVerbose(err_code);
    }
    if (!msg_suffix.empty())
        std::cout << " " << msg_suffix;
    std::cout << std::endl;
}