import unittest

from serial import Serial

import blipper

TTY = '/dev/slave'


class TestBlipper(unittest.TestCase):

    def setUp(self):
        self.ser = Serial(TTY, blipper.BAUD_RATE)

    def test_ping_ok(self):
        ping = blipper.Ping()
        self.ser.write(bytes(ping))
        packet = blipper.get_packet(self.ser)
        self.assertTrue(isinstance(packet, blipper.Pong))

    def tearDown(self):
        self.ser.close()


if __name__ == '__main__':
    unittest.main()
