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
void rotate(cv::Mat& frame)
{
    cv::Mat dst(frame.cols, frame.rows, frame.type());

    for (int y = 0; y < frame.rows; ++y)
        for (int x = 0; x < frame.cols; ++x)
            dst.at<cv::Vec3b>(x, y) = frame.at<cv::Vec3b>(frame.rows - y, x);

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

    // Retrieve
    int camIndex = 0;
    vector<string> inFiles;
    for (auto& cam : cameras)
    {
        cv::Mat frame = cam->retrieve();
        rotate(frame);
        string filename = GRAB_BASE_NAME + to_string(camIndex) + ".png";
        cv::imwrite(filename, frame);
        inFiles.push_back(filename);
        camIndex++;
    }

    // Wait for it, SD card can be _slow_
    this_thread::sleep_for(chrono::seconds(2));

    // Assemble
    auto cmd = string("/usr/local/bin/apngasm");
    vector<string> outArgs {"-F", "-d", "400", "-o"};
    auto outFile = string("output.png");
    char* args[] = {(char*)cmd.c_str(),
                  (char*)outArgs[3].c_str(),
                  (char*)outFile.c_str(),
                  (char*)inFiles[0].c_str(),
                  (char*)inFiles[1].c_str(),
                  (char*)inFiles[2].c_str(),
                  (char*)inFiles[3].c_str(),
                  (char*)inFiles[2].c_str(),
                  (char*)inFiles[1].c_str(),
                  (char*)outArgs[0].c_str(),
                  (char*)outArgs[1].c_str(),
                  (char*)outArgs[2].c_str(),
                  NULL};
    char* env[] = {NULL};

    int pid;
    int status = posix_spawn(&pid, cmd.c_str(), NULL, NULL, args, env);
    if (status == 0)
        waitpid(pid, nullptr, 0);
}
