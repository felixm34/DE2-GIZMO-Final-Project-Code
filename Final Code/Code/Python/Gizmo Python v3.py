#!/usr/bin/python2.7
import sys
sys.path.append('C:\\LeapSDK\\lib')
sys.path.append('C:\\LeapSDK\\lib\\x86') # Leap Motion SDK runs only in Python 2

import Leap, thread, time, serial, colorsys
import serial.tools.list_ports
from Leap import Finger # import useful functions from configured LeapSDK library

# auto detect Arduino Serial Port and Configure
for p in list(serial.tools.list_ports.comports()):
    if "Arduino" in p[1]:
        PORT = p[0]
        print "Arduino on port: %s" %PORT
BAUD = 9600

class SampleListener(Leap.Listener):
    finger_names = ['Thumb', 'Index', 'Middle', 'Ring', 'Pinky']
    ser = serial.Serial(PORT, BAUD)
    frame_cnt = 0

    def on_init(self, controller):
        print "Initialized"

    def on_connect(self, controller):
        print "Connected"

    def on_disconnect(self, controller):
        print "Disconnected"

    def on_exit(self, controller):
        print "Exited"

    def on_frame(self, controller):
        # get the most recent frame and report some basic information
        frame = controller.frame()
        self.frame_cnt += 1

        if self.frame_cnt % 2 == 0:
            # get hands
            for hand in frame.hands:

                handType = "Left hand" if hand.is_left else "Right hand"

                is_pointing = True if hand.grab_strength == 0.0 else False

                print "  %s, id %d, pointing: %s" % (
                    handType, hand.id, is_pointing)

                # get fingers
                index_fingers = hand.fingers.finger_type(Finger.TYPE_INDEX)
                for finger in index_fingers:

                    tpos = finger.tip_position # discover index finger tip position

                    print "   %s finger, tip: %s" % (
                        self.finger_names[finger.type],
                        tpos)

                    if is_pointing:
                        rgb = self.pos_to_rgb(tpos.x, tpos.y, tpos.z)
                        self.send_rgb(rgb)


    def pos_to_rgb(self, px, py, pz):
        # encode positional data to useful 8-bit values
        hue = (0.6375 * px + 127.5) % 255.0

        sat = 255.0 #pz

        if py < 25.0:
            lum = 0.0
        elif py > 380.0:
            lum = 255.0
        else:
            lum = 0.5595 * py -13.546

        return self.hsl_to_rgb(hue, sat, lum)

    def hsl_to_rgb(self, hue, sat, lum):
        rgb_dec = colorsys.hls_to_rgb(hue/255.0, lum/255.0, sat/255.0)
        rgb = [255.0 * i for i in rgb_dec]
        return rgb

    def send_rgb(self, rgb):
        cmd_str = '<' + str(map(int, rgb))[1:-1] + '>' + '\n'
        # output has form <x, y, z> to make Serial parsing easier
        print "       cmd_str: %s" % cmd_str
        self.ser.write(cmd_str.encode())

def main():
    # create a sample listener and controller
    listener = SampleListener()
    controller = Leap.Controller()

    # have the sample listener receive events from the controller
    controller.add_listener(listener)

    # keep this process running until Enter is pressed
    print "Press Enter to quit..."

    try:
        sys.stdin.readline()
    except KeyboardInterrupt:
        pass
    finally:
        # remove the sample listener when done
        controller.remove_listener(listener)

if __name__ == "__main__":
    main()
