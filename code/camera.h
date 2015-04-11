#include <memory>

#include <opencv2/opencv.hpp>

/*************/
class Camera
{
    public:
        Camera();
        ~Camera();

        bool connect(int index);
        bool grab();
        cv::Mat retrieve();

    private:
        std::unique_ptr<cv::VideoCapture> _camera;
        cv::Mat _frame;
        int _cameraIndex {-1};
};
