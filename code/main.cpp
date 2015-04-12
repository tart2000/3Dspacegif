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
void rotateAndShift(cv::Mat& frame, int shift = 0)
{
    cv::Mat dst(frame.cols, frame.rows, frame.type());

    for (int y = 0; y < frame.rows; ++y)
        for (int x = shift; x < frame.cols; ++x)
            dst.at<cv::Vec3b>(x, y) = frame.at<cv::Vec3b>(frame.rows - y, x - shift);

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
    vector<int> cameraShift {6, 28, 19, 0};
    int camIndex = 0;
    vector<string> inFiles;
    vector<cv::Mat> frames;
    for (auto& cam : cameras)
    {
        cv::Mat frame = cam->retrieve();
        rotateAndShift(frame, cameraShift[camIndex]);
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

    // Assemble
    //auto cmd = string("/usr/local/bin/apngasm");
    //vector<string> outArgs {"-F", "-d", "400", "-o"};
    //auto outFile = string("output.png");
    //char* args[] = {(char*)cmd.c_str(),
    //              (char*)outArgs[3].c_str(),
    //              (char*)outFile.c_str(),
    //              (char*)inFiles[0].c_str(),
    //              (char*)inFiles[1].c_str(),
    //              (char*)inFiles[2].c_str(),
    //              (char*)inFiles[3].c_str(),
    //              (char*)inFiles[2].c_str(),
    //              (char*)inFiles[1].c_str(),
    //              (char*)outArgs[0].c_str(),
    //              (char*)outArgs[1].c_str(),
    //              (char*)outArgs[2].c_str(),
    //              NULL};
    //char* env[] = {NULL};

    //int pid;
    //int status = posix_spawn(&pid, cmd.c_str(), NULL, NULL, args, env);
    //if (status == 0)
    //    waitpid(pid, nullptr, 0);
}
