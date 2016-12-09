#!/bin/bash
../../imageListCreator list.xml *.jpg
../../calibrator -w 9 -h 6 list.xml
