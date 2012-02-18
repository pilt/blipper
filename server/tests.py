import unittest

from serial import Serial

import blipper
from blipper import BAUD_RATE

TTY = '/dev/slave'


class TestBlipper(unittest.TestCase):

    def setUp(self):
        self.ser = Serial(TTY, blipper.BAUD_RATE)

    def test_ping_ok(self):
        ping = blipper.PingCommand()
        packet = bytes(ping)
        self.ser.write(packet)

    def tearDown(self):
        self.ser.close()


if __name__ == '__main__':
    unittest.main()
