#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include <opencv2/opencv.hpp>

#include "AnimatedGifSaver.h"

using namespace std;

/*************/
// Some global objects, yay!
unsigned int _cameraIndex {300};
float _timePerFrame {0.1f};
float _fov {40.0f};

cv::VideoCapture _camera;
chrono::system_clock::time_point _startTime;
cv::Mat _cameraMatrix;
cv::Mat _cameraDistCoeffs;

// Distortion correction
bool _distortionMapComputed {false};
cv::Mat _camMap1, _camMap2;
bool _conversionMapComputed {false};
cv::Mat _equMap;
bool _doConvertRectToEqui {true};

bool _onlyView {false};
bool _drawLines {false};
bool _onlyGrab {false};
bool _grabViews {false};
bool _grabCorrectedViews {false};
unsigned int _grabViewsPeriod {30};

// Cropping parameters
cv::Rect _cropView {0, 200, 1280, 720};

// Views related parameters
//vector<double> _viewLimits {0.0, 0.2595, 0.5, 0.7405, 1.0};
//vector<int> _xMove {58, 48, 0, -9};
//vector<int> _yMove {9, 5, 0, 1};
vector<double> _viewLimits {0.0, 0.25, 0.5, 0.75, 1.0};
vector<int> _xMove {84, 67, 0, -10};
//vector<int> _xMove {42, 32, 0, -5};
vector<int> _yMove {3, 13, 0, 2};

bool _viewDistortionMapsComputed {false};
vector<pair<cv::Mat, cv::Mat>> _viewMaps;

/*************/
void parseArgs(int argc, char** argv)
{
    for (unsigned int i = 0; i < argc;)
    {
        if (string(argv[i]) == "--grab")
        {
            _onlyGrab = true;
            ++i;
        }
        else if (string(argv[i]) == "--view")
        {
            _onlyView = true;
            ++i;
        }
        else if (string(argv[i]) == "--lines")
        {
            _drawLines = true;
            ++i;
        }
        else if (string(argv[i]) == "--grabViews")
        {
            _grabViews = true;
            ++i;
        }
        else if (string(argv[i]) == "--grabCorrectedViews")
        {
            _grabCorrectedViews = true;
            ++i;
        }
        else if (string(argv[i]) == "--cam" && i < argc - 1)
        {
            _cameraIndex = stoi(string(argv[i + 1]));
            i += 2;
        }
        else if (string(argv[i]) == "--time" && i < argc - 1)
        {
            _timePerFrame = stof(string(argv[i + 1]));
            i += 2;
        }
        else if (string(argv[i]) == "-h")
        {
            cout << "3dbox, to be used with the 3d mirror box" << endl;
            cout << "Usage:" << endl;
            cout << "  --cam : set the camera index to use (default: 300)" << endl;
            cout << "  --time : set the time between frames (default: 0.05sec)" << endl;
            cout << "  --grab : save images right after the grab, then loop" << endl;
            cout << "  --grabViews : save images right after the four views splitting, then loop" << endl;
            exit(0);
        }
        else
            ++i;
    }
}

/*************/
void moveImage(cv::Mat& img, int x, int y)
{
    auto correctedImage = img.clone();

    for (int i = 0; i < img.cols; ++i)
    {
        if (i - x < 0)
            continue;
        if (i - x >= img.cols)
            continue;

        for (int j = 0; j < img.rows; ++j)
        {
            if (j - y < 0)
                continue;
            if (j - y >= img.rows)
                continue;

            correctedImage.at<cv::Vec3b>(j - y, i - x) = img.at<cv::Vec3b>(j, i);
        }
    }

    img = correctedImage;
}

/*************/
int main(int argc, char** argv)
{
    // Initialization
    parseArgs(argc, argv);
    _startTime = chrono::system_clock::now();

    // Camera allocation
    _camera.open(_cameraIndex);
    if (!_camera.isOpened())
    {
        cout << "Could not open camera. Exiting." << endl;
        exit(0);
    }
    _camera.set(CV_CAP_PROP_ISO_SPEED, 800);
    _camera.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    _camera.set(CV_CAP_PROP_FRAME_HEIGHT, 960);
    cout << "Camera successfully initialized" << endl;

    {
        cv::FileStorage fileIntrinsic;
        if (!fileIntrinsic.open("camera_intrinsic.yml", cv::FileStorage::READ))
        {
            cout << "Could not load camera intrisic parameter. Exiting." << endl;
            exit(0);
        }
        fileIntrinsic["camera_matrix"] >> _cameraMatrix;
        fileIntrinsic["distortion_coefficients"] >> _cameraDistCoeffs;
    }

    auto continueLoop = true;
    while (continueLoop)
    {
        auto now = chrono::system_clock::now();
        int timestamp = (now - _startTime).count() / 1e6;

        // For simplicity, grab key here
        short key = cv::waitKey(1);
        auto saveGif = false;
        switch (key)
        {
        default:
            if (key != -1)
                cout << "Pressed key: " << key << endl;
            break;
        case 27: // Escape
            continueLoop = false;
            break;
        case 'g': // Save the gif
            saveGif = true;
            break;
        case 'r': // Switcher rect to equi conversion
            _doConvertRectToEqui = !_doConvertRectToEqui;
        }

        // Capture
        _camera.grab();
        auto rawImage = cv::Mat();
        _camera.retrieve(rawImage);
        auto rawSize = rawImage.size();

        // Save to files of only grabbing
        if (_onlyGrab)
        {
            string filename = "grabs/rawImage_" + to_string(timestamp) + ".jpg";
            cv::imwrite(filename, rawImage, {CV_IMWRITE_JPEG_QUALITY, 95});
            continue;
        }

        // Correct the camera grab for lens deformation
        if (!_distortionMapComputed)
        {
            _distortionMapComputed = true;
            initUndistortRectifyMap(_cameraMatrix, _cameraDistCoeffs, cv::Mat(), 
                getOptimalNewCameraMatrix(_cameraMatrix, _cameraDistCoeffs, rawSize, 0, rawSize, 0),
                rawSize, CV_16SC2, _camMap1, _camMap2);
        }

        auto correctedImage = cv::Mat();
        cv::remap(rawImage, correctedImage, _camMap1, _camMap2, cv::INTER_LINEAR);

        // Convert the image from rectilinear to equirectangular
        if (!_conversionMapComputed)
        {
            int width = correctedImage.cols;
            int height = correctedImage.rows;
            _equMap = cv::Mat::zeros(height, width, CV_32FC2);

            float radPerPix = _fov / width / 180.0 * M_PI;
            float sinWidth = sin(_fov * M_PI / 360.f);
            float sinHeight = sin(_fov * height / width * M_PI / 360.f);
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    float yaw = (float)(x - (width / 2)) * radPerPix;
                    float pitch = (float)(y - (height / 2)) * radPerPix;

                    float targetX = sin(yaw) / sinWidth * (float)(width / 2) + (float)(width / 2);
                    float equivalentFocal = 1.f / cos(yaw);
                    float targetY = equivalentFocal * sin(pitch) / sinHeight * (float)(height / 2) + (float)(height / 2);

                    _equMap.at<cv::Vec2f>(y, x)[0] = targetX;
                    _equMap.at<cv::Vec2f>(y, x)[1] = targetY;
                }
            }

            _conversionMapComputed = true;
        }

        if (_doConvertRectToEqui)
        {
            auto tmpImage = cv::Mat();
            cv::remap(correctedImage, tmpImage, _equMap, cv::Mat(), cv::INTER_LINEAR);
            correctedImage = tmpImage;
        }

        // Crop the image
        {
            auto tmpImage = cv::Mat(correctedImage, _cropView);
            correctedImage = tmpImage;
            rawSize = cv::Size(_cropView.width, _cropView.height);
        }

        // Draw separations between views
        if (_drawLines)
        {
            auto width = correctedImage.cols;
            auto height = correctedImage.rows;
            cv::line(correctedImage, cv::Point(width * _viewLimits[1], 0), cv::Point(width * _viewLimits[1], height), cv::Scalar(127, 0, 0), 2);
            cv::line(correctedImage, cv::Point(width * _viewLimits[2], 0), cv::Point(width * _viewLimits[2], height), cv::Scalar(127, 0, 0), 2);
            cv::line(correctedImage, cv::Point(width * _viewLimits[3], 0), cv::Point(width * _viewLimits[3], height), cv::Scalar(127, 0, 0), 2);

            cv::line(correctedImage, cv::Point(width * (_viewLimits[0] + _viewLimits[1]) / 2.f, 0),
                                     cv::Point(width * (_viewLimits[0] + _viewLimits[1]) / 2.f, height), cv::Scalar(0, 127, 0), 1);
            cv::line(correctedImage, cv::Point(width * (_viewLimits[1] + _viewLimits[2]) / 2.f, 0),
                                     cv::Point(width * (_viewLimits[1] + _viewLimits[2]) / 2.f, height), cv::Scalar(0, 127, 0), 1);
            cv::line(correctedImage, cv::Point(width * (_viewLimits[2] + _viewLimits[3]) / 2.f, 0),
                                     cv::Point(width * (_viewLimits[2] + _viewLimits[3]) / 2.f, height), cv::Scalar(0, 127, 0), 1);
            cv::line(correctedImage, cv::Point(width * (_viewLimits[3] + _viewLimits[4]) / 2.f, 0),
                                     cv::Point(width * (_viewLimits[3] + _viewLimits[4]) / 2.f, height), cv::Scalar(0, 127, 0), 1);
        }

        cv::imshow("Input corrected", correctedImage);
        if (_onlyView)
            continue;

        // Separate in 4 parts
        vector<cv::Mat> viewImages;
        for (unsigned int i = 0; i < 4; ++i)
        {
            auto roi = cv::Rect((int)(_viewLimits[i] * rawSize.width), 0, (int)((_viewLimits[i+1] - _viewLimits[i]) * rawSize.width), rawSize.height);
            auto viewImage = cv::Mat(correctedImage, roi);
            viewImages.push_back(viewImage);
        }

        // Save the four grabs
        if (_grabViews)
        {
            static unsigned int moment = 0;
            moment = (moment + 1) % _grabViewsPeriod;
            if (moment != 0)
                continue;

            for (unsigned int i = 0; i < 4; ++i)
            {
                string filename = "grabs/viewImage_" + to_string(i) + "_" + to_string(timestamp) + ".jpg";
                cv::imwrite(filename, viewImages[i], {CV_IMWRITE_JPEG_QUALITY, 95});
            }
            continue;
        }

        // Correct the different grabs
        // First compute the distortion maps
        if (!_viewDistortionMapsComputed)
        {
            for (unsigned int i = 0; i < 4; ++i)
            {
                cv::FileStorage fileIntrinsic;
                string filename = "view_" + to_string(i) + "_intrinsic.yml";
                if (!fileIntrinsic.open(filename, cv::FileStorage::READ))
                {
                    cout << "Could not load view " << i << " intrinsic parameters. Exiting" << endl;
                    exit(0);
                }
                auto camMatrix = cv::Mat();
                auto distCoeffs = cv::Mat();
                fileIntrinsic["camera_matrix"] >> camMatrix;
                fileIntrinsic["distortion_coefficients"] >> distCoeffs;

                auto map1 = cv::Mat();
                auto map2 = cv::Mat();
                initUndistortRectifyMap(camMatrix, distCoeffs, cv::Mat(), 
                    getOptimalNewCameraMatrix(camMatrix, distCoeffs, viewImages[i].size(), 0, viewImages[i].size(), 0),
                    viewImages[i].size(), CV_16SC2, map1, map2);

                _viewMaps.push_back(make_pair(map1, map2));
            }
            _viewDistortionMapsComputed = true;
        }

        // Apply the correction
        vector<cv::Mat> _correctedViews;
        for (unsigned int i = 0; i < 4; ++i)
        {
            auto correctedView = cv::Mat();
            auto tmpView = viewImages[i].clone();

            //cv::remap(viewImages[i], correctedView, _viewMaps[i].first, _viewMaps[i].second, cv::INTER_LINEAR);
            //cv::flip(correctedView, tmpView, 0);
            //cv::cvtColor(tmpView, correctedView, CV_BGR2RGB);
            //moveImage(correctedView, _xMove[i], _yMove[i]);
            //_correctedViews.push_back(correctedView);

            cv::flip(viewImages[i], tmpView, 0);
            cv::cvtColor(tmpView, correctedView, CV_BGR2RGB);
            moveImage(correctedView, _xMove[i], 0);
            _correctedViews.push_back(correctedView);
        }

        // Save the four corrected views
        if (_grabCorrectedViews)
        {
            static unsigned int moment = 0;
            moment = (moment + 1) % _grabViewsPeriod;
            if (moment != 0)
                continue;

            for (unsigned int i = 0; i < 4; ++i)
            {
                string filename = "grabs/viewImage_" + to_string(i) + "_" + to_string(timestamp) + ".jpg";
                cv::imwrite(filename, _correctedViews[i], {CV_IMWRITE_JPEG_QUALITY, 95});
            }
            continue;
        }

        // Save the gif
        if (saveGif)
        {
            cout << "Grab a 3d GIF!" << endl;
            // Get the largest input view
            auto finalSize = cv::Size(0, 0);
            for (auto& view : _correctedViews)
            {
                finalSize.width = std::max(finalSize.width, view.cols);
                finalSize.height = std::max(finalSize.height, view.rows);
            }
            
            // Resize and crop all views
            for (auto& view : _correctedViews)
            {
                auto resized = cv::Mat();
                cv::resize(view, resized, finalSize);
                view = cv::Mat(resized, cv::Rect(0, 0, finalSize.width - 96, finalSize.height)).clone();
            }
            finalSize.width -= 96;

            // Save input files
            for (unsigned int i = 0; i < 4; ++i)
                cv::imwrite("gifs/src_" + to_string(i) + ".png", _correctedViews[i]);


            auto gifSaver = AnimatedGifSaver(finalSize.width, finalSize.height);

            // Set the views for the gif saver
            //for (auto& view : _correctedViews)
            //    gifSaver.AddFrame(view.data, _timePerFrame);
            // Add two frames to make a correct loop
            gifSaver.AddFrame(_correctedViews[0].data, _timePerFrame * 1.5);
            gifSaver.AddFrame(_correctedViews[1].data, _timePerFrame * 0.8);
            gifSaver.AddFrame(_correctedViews[2].data, _timePerFrame * 0.8);
            gifSaver.AddFrame(_correctedViews[3].data, _timePerFrame * 1.5);
            gifSaver.AddFrame(_correctedViews[2].data, _timePerFrame * 0.8);
            gifSaver.AddFrame(_correctedViews[1].data, _timePerFrame * 0.8);

            string filename = "gifs/shot_" + to_string(timestamp) + ".gif";
            gifSaver.Save(filename.c_str());
        }
    }
}
