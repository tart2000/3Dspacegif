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
        _camera->release();
        _camera = nullptr;
        _cameraIndex = index;
        return true;
    }
}

/*************/
bool Camera::grab()
{
    _camera.reset(new cv::VideoCapture(_cameraIndex));
    if (_camera->isOpened())
    {
        cv::Mat frame;
        bool res;
        res = _camera->grab();
        res &= _camera->retrieve(frame);
        if (res)
            _frame = frame.clone();
        else
            cout << "ERROR" << endl;

        _camera->release();
        _camera = nullptr;

        return res;
    }
    else
        return false;
}

/*************/
cv::Mat Camera::retrieve()
{
    return _frame;
}
