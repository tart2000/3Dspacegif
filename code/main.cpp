#include <iostream>
#include <memory>
#include <vector>

#include <opencv2/opencv.hpp>
#include "camera.h"

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
}
