#!/bin/bash
g++ -O2 -std=c++11 -o 3dbox main.cpp AnimatedGifSaver.cpp `pkg-config --cflags --libs opencv` -lgif
