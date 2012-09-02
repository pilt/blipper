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
        rows = [[1] * 128] * 64
        redraw = blipper.RedrawPixels.from_matrix(display, rows)
        waiter = Waiter(bytes(redraw), 'ok')

        def on_redraw(packet):
            waiter.got(bytes(packet))
        self.slave.on(blipper.RedrawPixels, on_redraw)

        def on_ok(packet):
            waiter.got('ok')
        self.master.on(blipper.Ok, on_ok)

        self.master.send(redraw)
        waiter.wait(1.0)
        self.assertTrue(waiter.has_all())

    def test_buy_ok(self):
        button = 3
        card_id = 2**32 - 5
        buy = blipper.Buy.from_button_and_card_id(button, card_id)
        new_balance = 5000
        waiter = Waiter(button, new_balance, card_id)

        def on_buy(packet):
            waiter.got(packet.get_button())
            waiter.got(packet.get_card_id())
            return blipper.NewBalance.from_new_balance(new_balance)
        self.slave.on(blipper.Buy, on_buy)

        def on_buy_ok(packet):
            waiter.got(packet.get_new_balance())
        self.master.on(blipper.NewBalance, on_buy_ok)

        self.master.send(buy)
        waiter.wait(1.0)
        self.assertTrue(waiter.has_all())

    def test_insufficient_funds(self):
        button = 2
        card_id = 2**32 - 5
        buy = blipper.Buy.from_button_and_card_id(button, card_id)
        waiter = Waiter(button, 1337, card_id)

        def on_buy(packet):
            waiter.got(packet.get_button())
            waiter.got(packet.get_card_id())
            return blipper.InsufficientFunds.from_balance(1337)
        self.slave.on(blipper.Buy, on_buy)

        def on_insufficient_fund(packet):
            waiter.got(packet.get_balance())
        self.master.on(blipper.InsufficientFunds, on_insufficient_fund)

        self.master.send(buy)
        waiter.wait(1.0)
        self.assertTrue(waiter.has_all())

    def test_invalid_card_id(self):
        button = 2
        card_id = 2**32 - 5
        invalid_card = blipper.InvalidCardId()
        buy = blipper.Buy.from_button_and_card_id(button, card_id)
        waiter = Waiter(button, card_id, bytes(invalid_card))

        def on_buy(packet):
            waiter.got(packet.get_button())
            waiter.got(packet.get_card_id())
            return invalid_card
        self.slave.on(blipper.Buy, on_buy)

        def on_invalid_card_id(packet):
            waiter.got(bytes(packet))
        self.master.on(blipper.InvalidCardId, on_invalid_card_id)

        self.master.send(buy)
        waiter.wait(1.0)
        self.assertTrue(waiter.has_all())

    def test_next_shift(self):
        button = 2
        card_id = 2**32 - 5
        next_shift = blipper.NextShift.from_text(u'fm 2012-04-23')
        buy = blipper.Buy.from_button_and_card_id(button, card_id)
        waiter = Waiter(button, card_id, bytes(next_shift))

        def on_buy(packet):
            waiter.got(packet.get_button())
            waiter.got(packet.get_card_id())
            return next_shift
        self.slave.on(blipper.Buy, on_buy)

        def on_next_shift(packet):
            waiter.got(bytes(packet))
        self.master.on(blipper.NextShift, on_next_shift)

        self.master.send(buy)
        waiter.wait(1.0)
        self.assertTrue(waiter.has_all())

    def test_multiple_buys(self):
        button_responses = {
            0: blipper.NewBalance.from_new_balance(0),
            1: blipper.NewBalance.from_new_balance(1),
            2: blipper.InsufficientFunds.from_balance(1337),
        }

        button_presses = [0, 1, 2]
        card_id = 2**32 - 5
        from_func = blipper.Buy.from_button_and_card_id
        buys = [from_func(btn, card_id) for btn in button_presses]
        waiter = Waiter(
                'b0',
                'b1',
                'b2',
                'nb0',
                'nb1',
                'i1337',
                "c%s-0" % card_id,
                "c%s-1" % card_id,
                "c%s-2" % card_id)

        def on_buy(packet):
            got_button = packet.get_button()
            waiter.got('b%s' % got_button)
            waiter.got('c%s-%s' % (packet.get_card_id(), got_button))
            return button_responses[got_button]

        self.slave.on(blipper.Buy, on_buy)

        def on_buy_ok(packet):
            waiter.got('nb%s' % packet.get_new_balance())
        self.master.on(blipper.NewBalance, on_buy_ok)

        def on_insufficient(packet):
            waiter.got('i%s' % packet.get_balance())
        self.master.on(blipper.InsufficientFunds, on_insufficient)

        map(self.master.send, buys)
        waiter.wait(1.0)
        self.assertTrue(waiter.has_all())


if __name__ == '__main__':
    unittest.main()
