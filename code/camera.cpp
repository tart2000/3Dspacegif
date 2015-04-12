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
        _cameraIndex = -1;
        return false;
    }
    else
    {
        _cameraIndex = index;
        return true;
    }
}

/*************/
bool Camera::grab()
{
    if (_camera->isOpened())
    {
        cv::Mat frame;
        bool res;
        res = _camera->read(frame);
        if (res)
            _frame = frame.clone();
        else
            cout << "ERROR" << endl;

        _camera->release();

        return res;
    }
    else
        return false;
}

/*************/
cv::Mat Camera::retrieve()
{
    _camera->open(_cameraIndex);
    return _frame;
}
