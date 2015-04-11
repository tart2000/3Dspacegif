#include <iostream>
#include <memory>
#include <spawn.h>
#include <string>
#include <sys/wait.h>
#include <vector>

#include <opencv2/opencv.hpp>
#include "camera.h"

#define GRAB_BASE_NAME "/tmp/capture_"

using namespace std;

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
    for (auto& cam : cameras)
    {
        cv::Mat frame = cam->retrieve();
        cv::imwrite(GRAB_BASE_NAME + to_string(camIndex) + ".png", frame);
        camIndex++;
    }

    // Assemble
    auto cmd = string("/usr/bin/apngasm");
    auto outFile = string("output.png");
    auto inFile = string("/tmp/capture_*.png");
    char* args[] = {(char*)cmd.c_str(),
                  (char*)outFile.c_str(),
                  (char*)inFile.c_str(),
                  NULL};
    char* env[] = {NULL};

    int pid;
    int status = posix_spawn(&pid, cmd.c_str(), NULL, NULL, args, env);
    cout << "---> " << pid << " -- " << status << endl;
    if (status == 0)
        waitpid(pid, nullptr, 0);
}
