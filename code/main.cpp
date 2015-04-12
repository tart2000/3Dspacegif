#include <chrono>
#include <iostream>
#include <memory>
#include <spawn.h>
#include <string>
#include <sys/wait.h>
#include <thread>
#include <vector>

#include <opencv2/opencv.hpp>
#include "camera.h"

#define GRAB_BASE_NAME "/tmp/capture_"

using namespace std;

/*************/
void rotateAndShift(cv::Mat& frame, int shiftx = 0, int shifty = 0)
{
    cv::Mat dst(frame.cols, frame.rows, frame.type());

    for (int y = std::max(0, shifty); y < frame.rows + std::min(0, shifty); ++y)
        for (int x = std::max(0, shiftx); x < frame.cols + std::min(0, shiftx); ++x)
            dst.at<cv::Vec3b>(x, y) = frame.at<cv::Vec3b>(frame.rows - y + shifty, x - shiftx);

    frame = dst;
}

/*************/
int main(int argc, char** argv)
{
    vector<unique_ptr<Camera>> cameras;
    for (int i = 0; i < 4; ++i)
    {
        unique_ptr<Camera> cam(new Camera);
        bool res = cam->connect(i);
        if (res)
            cameras.push_back(move(cam));
        else
            break;
    }

    cout << "Number of connected cameras: " << cameras.size() << endl;

    // Grab
    for (auto& cam : cameras)
    {
        if (!cam->grab())
            cout << "Error grabbing on one camera" << endl;
    }

    // Retrieve, rotate and shift
    //vector<int> cameraShiftX {6, 28, 19, 0};
    vector<int> cameraShiftX {-29, 0, -14, -26};
    //vector<int> cameraShiftY {-45, 0, 76, 98};
    vector<int> cameraShiftY {-40, 0, 70, 90};
    int camIndex = 0;
    vector<string> inFiles;
    vector<cv::Mat> frames;
    for (auto& cam : cameras)
    {
        cv::Mat frame = cam->retrieve();
        rotateAndShift(frame, cameraShiftX[camIndex], cameraShiftY[camIndex]);
        //cv::Mat cropped = frame(cv::Rect(98, 0, 337, 611)).clone();
        cv::Mat cropped = frame(cv::Rect(98, 80, 337, 449)).clone();
        cv::resize(cropped, frame, cv::Size(480, 640));
        string filename = GRAB_BASE_NAME + to_string(camIndex) + ".png";
        cv::imwrite(filename, frame);
        inFiles.push_back(filename);
        frames.push_back(frame);
        camIndex++;
    }

    // Save copies of the two intermediate images
    cv::imwrite(GRAB_BASE_NAME + string("4.png"), frames[2]);
    cv::imwrite(GRAB_BASE_NAME + string("5.png"), frames[1]);

    // Wait for it, SD card can be _slow_
    this_thread::sleep_for(chrono::seconds(2));
}
