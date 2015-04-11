#include "camera.h"

#include <iostream>

using namespace std;

/*************/
Camera::Camera()
{
}

/*************/
Camera::~Camera()
{
}

/*************/
bool Camera::connect(int index)
{
    _camera.reset(new cv::VideoCapture(index));
    if (!_camera->isOpened())
    {
        _camera = nullptr;
        return false;
    }
    else
        return true;
}

/*************/
bool Camera::grab()
{
    return _camera->grab();
}

/*************/
cv::Mat Camera::retrieve()
{
    cv::Mat frame;
    bool res = _camera->retrieve(frame);
    _frame = frame;
    return frame;
}
