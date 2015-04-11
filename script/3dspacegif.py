#!/usr/bin/env python

import io, time
import RPi.GPIO as GPIO
import subprocess

def mainLoop():
    # Deactivate autofocus for all cameras
    cameraDev = ["/dev/video0",
                 "/dev/video1",
                 "/dev/video2",
                 "/dev/video3"]

    cameraFocus = ["300",
                   "400",
                   "500",
                   "350"]

    #for dev in cameraDev:
    for idx in range(0, 4):
        command = "v4l2-ctl -d " + cameraDev[idx] + " --set-ctrl=focus_auto=0"
        print(command)
        process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
        output = process.communicate()[0]
        print(output)

        command = "v4l2-ctl -d " + cameraDev[idx] + " --set-ctrl=focus_absolute=" + cameraFocus[idx]
        print(command)
        process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
        output = process.communicate()[0]
        print(output)

    GPIO.setmode(GPIO.BCM)
    GPIO.setup(18, GPIO.IN, pull_up_down=GPIO.PUD_UP)

    while True: 
        inputState = GPIO.input(18)
        # If the button has been pressed
        if inputState == 0:
            command = "./code/3dbox"
            process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
            output = process.communicate()[0]
            print(output)

    GPIO.cleanup()

mainLoop()
