#!/bin/bash
g++ -std=c++11 -o 3dbox main.cpp camera.cpp `pkg-config --cflags --libs opencv`
