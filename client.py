import serial
import time
import sys
from cmd import Cmd
from PIL import ImageGrab, ImageStat
from itertools import *
from struct import pack

#------------------------------------------------------------------------------
# Configuration
#------------------------------------------------------------------------------

WIDTH = 29
HEIGHT = 17
COM_PORT = 'COM3'

#------------------------------------------------------------------------------
# Protocol
#------------------------------------------------------------------------------

ESCAPE = '\x13'
START = '\x37'

COMMAND_HELLO = '\x01'
COMMAND_LEDS = '\x02'


def escape(data):
    return data.replace(ESCAPE, ESCAPE + ESCAPE)

def command_hello():
    ret = ESCAPE + START + COMMAND_HELLO
    return ret

def command_leds(leds):
    ret = ESCAPE + START + COMMAND_LEDS + chr(len(leds))
    for led in leds:
        ret += escape(led)
    return ret

#------------------------------------------------------------------------------
# Screen monitor
#------------------------------------------------------------------------------

def mask(image, w, h, x, y):
    cell_w = 1.0 / float(w)
    cell_h = 1.0 / float(h)
    left = x * cell_w
    right = left + cell_w
    top = y * cell_h
    bottom = top + cell_h
    im_w, im_h = image.size
    return (int(left * im_w), int(top * im_h), int(right * im_w), int(bottom * im_h))

def get_outline_indices(w, h, blank_corners=False):
    outline = []
    outline += [(0, y-1) for y in xrange(h, 0, -1)]
    outline += [(-1, -1)]
    outline += [(x, 0) for x in xrange(w)]
    outline += [(-1, -1)]
    outline += [(w-1, y) for y in xrange(h)]
    return outline

INDICES = get_outline_indices(WIDTH, HEIGHT, True)

def get_outline(width, height):
    # NOTE: Windows only
    im = ImageGrab.grab()

    # scale down so its faster
    w, h = im.size
    im = im.resize((w/4, h/4))
    w, h = im.size

    res = []
    for x, y in INDICES:
        if x == -1 and y == -1:
            res += [(0,0,0)]
        else:
            st = ImageStat.Stat(im.crop(mask(im, width, height, x, y)))
            res += [st.mean]
    return res

def monitor_screen(ser):
    try:
        while True:
            leds = get_outline(WIDTH, HEIGHT)
            leds = [pack('BBB', rgb[1], rgb[0], rgb[2]) for rgb in leds]
            ser.write(command_leds(leds))

    except KeyboardInterrupt: pass

#------------------------------------------------------------------------------
# Command line handler
#------------------------------------------------------------------------------

class LedClient(Cmd):
    def __init__(self, *args, **kwargs):
        self.serial = None
        Cmd.__init__(self, *args, **kwargs)

    def do_connect(self, arg):
        if self.serial:
            self.do_close()
        self.serial = serial.Serial(COM_PORT, 115200, timeout=0)
        print 'Connected'

    def do_hello(self, arg):
        if not self.serial:
            print 'Not connected'
            return
        self.serial.write(command_hello())

    def do_monitor(self, arg):
        if not self.serial:
            print 'Not connected'
            return

        print 'Monitoring...'
        monitor_screen(self.serial)
        print 'Done'

    def do_close(self, arg):
        if not self.serial:
            return
        try:
            self.serial.close()
        finally:
            self.serial = None

    def do_quit(self, arg):
        return True
    do_q = do_quit


def main():
    ledclient = LedClient()
    ledclient.do_connect("")
    ledclient.do_monitor("")
    ledclient.cmdloop()
   

if __name__ == '__main__':
    main()
