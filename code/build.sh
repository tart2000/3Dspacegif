#!/bin/bash
g++-4.8 -O2 -std=c++11 -o 3dbox main.cpp camera.cpp `pkg-config --cflags --libs opencv`
