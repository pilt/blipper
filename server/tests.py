import logging
import unittest
import time

from serial import Serial

import blipper

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger('blipper.test')
master_logger = logging.getLogger('blipper.test.master')
slave_logger = logging.getLogger('blipper.test.slave')

MASTER_TTY = blipper.TTY
SLAVE_TTY = '/dev/slave'


class Waiter(object):

    def __init__(self, *wait_for):
        self._got = dict((w, False) for w in wait_for)

    def got(self, token):
        self._got[token] = True

    def has_all(self):
        return all(self._got.values())

    def wait(self, timeout=None):
        waited = 0.0
        sleep_time = 0.1
        while True:
            if self.has_all():
                break
            time.sleep(sleep_time)
            waited += sleep_time
            if timeout is not None and waited >= timeout:
                break



class TestBlipper(unittest.TestCase):

    def setUp(self):
        self.master = blipper.Thread(MASTER_TTY, logger=master_logger)
        self.master.start()
        self.slave = blipper.Thread(SLAVE_TTY, logger=slave_logger)
        self.slave.start()
        while True:
            time.sleep(0.1)
            if self.master.waiting_for_packet and self.slave.waiting_for_packet:
                break

    def tearDown(self):
        for server in [self.master, self.slave]:
            try:
                server.join()
            except:
                pass

    def test_ping_ok(self):
        ping = blipper.Ping()
        waiter = Waiter('ping', 'pong')

        def on_ping(packet):
            waiter.got('ping')
        self.slave.on(blipper.Ping, on_ping)

        def on_pong(packet):
            waiter.got('pong')
        self.master.on(blipper.Pong, on_pong)

        self.master.send(ping)
        waiter.wait(1.0)
        self.assertTrue(waiter.has_all())

    def test_redraw_ok(self):

        display = 0
        rows = [[1] * 16] * 8
        redraw = blipper.RedrawPixels.from_matrix(display, rows)
        waiter = Waiter(bytes(redraw))

        def on_redraw(packet):
            logger.info('got %s', bytes(packet))
            waiter.got(bytes(packet))
        self.slave.on(blipper.RedrawPixels, on_redraw)

        self.master.send(redraw)
        waiter.wait(1.0)
        self.assertTrue(waiter.has_all())


if __name__ == '__main__':
    unittest.main()
