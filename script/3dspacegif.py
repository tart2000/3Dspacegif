#!/usr/bin/env python

import io, time
import RPi.GPIO as GPIO
import subprocess
import time
import paramiko
import signal

continueLoop = True

def sigHandler(signum, frame):
    global continueLoop
    continueLoop = False

def mainLoop():
    global continueLoop

    signal.signal(signal.SIGTERM, sigHandler)
    signal.signal(signal.SIGQUIT, sigHandler)

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

    mplayerProcess = None

    # Calibrate the exposures
    command = "./code/3dbox"
    process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
    output = process.communicate()[0]

    print("READY TO GO")

    GPIO.setup(24, GPIO.OUT)
    GPIO.output(24, GPIO.HIGH)

    while continueLoop: 
        inputState = GPIO.input(18)
        # If the button has been pressed
        if inputState == 0:
            time.sleep(1)

            command = "./code/3dbox"
            process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
            output = process.communicate()[0]
            print(output)

            # Create the avi for mplayer
            #command = "avconv -r 6 -i /tmp/capture_%d.png -y -b:v 1000k -s 320x240 -vf transpose=1,transpose=1,transpose=1 /tmp/capture.avi"
            #convertProcess = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
            #convertProcess.communicate()[0]

            #time.sleep(1)

            # Play the avi
            #if mplayerProcess is not None:
            #    mplayerProcess.terminate()
            #command = "mplayer -vo fbdev2:/dev/fb1 -x 240 -y 320 -framedrop -loop 0 /tmp/capture.avi"
            #mplayerProcess = subprocess.Popen(command.split(), stdout=subprocess.PIPE)

            # Create the APNG
            command = "/usr/local/bin/apngasm -o output.png /tmp/capture_0.png /tmp/capture_1.png /tmp/capture_2.png /tmp/capture_3.png /tmp/capture_4.png /tmp/capture_5.png -F -d 200"
            apngProcess = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
            apngProcess.wait()

            # Send to the server
            host = "10.42.0.1"
            port = 22
            transport = paramiko.Transport((host, port))
            transport.connect(username="manu", password="mushroom")
            sftp = paramiko.SFTPClient.from_transport(transport)
            
            import sys
            localpath = './output.png'
            targetpath = './src/3dBox/space_gif_online/content/gifs/img_' + str(time.time()) + '.png'
            sftp.put(localpath, targetpath)
            
            sftp.close()
            transport.close
            print('Upload done')

            #mplayerProcess.terminate()
            #mplayerProcess = None

        time.sleep(0.016)

    GPIO.cleanup()

mainLoop()
