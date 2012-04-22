import sys
import struct
import re
import threading
import time
from collections import defaultdict
import logging
import datetime

from serial import Serial

TTY = '/dev/master'
BAUD_RATE = 9600
PREAMBLE = chr(0xaa) + chr(0x55)

logger = logging.getLogger('blipper')

class BlipperError(Exception): pass
class PacketError(BlipperError): pass
class MultipleResponses(BlipperError): pass


packet_types = {}

def packet_type(cls):
    packet_types[cls.cmd] = cls
    return cls


def get_checksum(buf):
    checksum = 0x00
    for byte in buf:
        checksum += ord(byte)
    return chr(checksum % (0xff + 1))


class Header(object):
    length = 6

    def __init__(self, cmd, body_length):
        self.cmd = cmd
        self.body_length = body_length


def unpack_header(buf):
    if len(buf) != Header.length:
        raise PacketError("invalid header length")
    fields = struct.unpack("!BBBHB", buf[:6])
    preamble = chr(fields[0]) + chr(fields[1])
    if preamble != PREAMBLE:
        raise PacketError("invalid preamble")
    cmd = chr(fields[2])
    length = fields[3]
    cs = chr(fields[4])
    cs_cmp = get_checksum(buf[:5])
    if cs != cs_cmp:
        raise PacketError("invalid header checksum")
    return Header(cmd, length)


class Packet(object):

    def __init__(self):
        self.body = ''

    @classmethod
    def from_body(cls, body):
        obj = cls()
        obj.body = body
        return obj

    def __str__(self):
        buf = PREAMBLE + self.cmd
        buf += struct.pack("!H", len(self.body))
        buf += get_checksum(buf)
        buf += self.body
        buf += get_checksum(self.body)
        return buf

    def __repr__(self):
        return '<%s>' % self.__class__.__name__


class Request(Packet):

    def default_response(self):
        return Ok()


class Response(Packet):

    def default_response(self):
        return None


@packet_type
class Ok(Response):
    cmd = chr(0x00)


@packet_type
class Fail(Response):
    cmd = chr(0x01)


@packet_type
class Ping(Request):
    cmd = chr(0x02)

    def default_response(self):
        return Pong()


@packet_type
class Pong(Response):
    cmd = chr(0x03)


@packet_type
class RedrawPixels(Request):
    """
    1 byte display index. A display has 128x64 pixels. Pixel data for one
    row is packed to 16 bytes (128/8). 1B + 16B * 64  = 1,025 bytes display 
    index and pixel data. 
    """
    cmd = chr(0x04)
    num_rows = 64
    num_cols = 128
    _byte_buf_pattern = re.compile("[01]{%s}" % 8)

    @classmethod
    def from_matrix(cls, display, rows):
        assert len(rows) == cls.num_rows 
        body_byte_ints = []
        for row in rows:
            assert len(row) == cls.num_cols
            for byte_start_idx in range(cls.num_cols)[::8]:
                byte_end_idx = byte_start_idx + 8
                buf = "".join(map(str, row[byte_start_idx:byte_end_idx]))
                if not cls._byte_buf_pattern.match(buf):
                    raise PacketError("invalid byte data")
                body_byte_ints.append(int(buf, 2))
        display_index_fmt = "B"
        row_fmt = "B" * (cls.num_cols // 8)
        fmt = "!" + display_index_fmt + row_fmt * cls.num_rows
        body = struct.pack(fmt, display, *body_byte_ints)
        return RedrawPixels.from_body(body)


@packet_type
class Buy(Request):
    cmd = chr(0x05)
    fmt = "!BI"

    @classmethod
    def from_button_and_card_id(cls, button, card_id):
        if 0 <= button <= 255 and 0 <= card_id <= 2**32 - 1:
            body = struct.pack(cls.fmt, button, card_id)
            return Buy.from_body(body)
        raise BlipperError(button)

    def get_button(self):
        return struct.unpack(self.fmt, self.body)[0]

    def get_card_id(self):
        return struct.unpack(self.fmt, self.body)[1]


@packet_type
class NewBalance(Response):
    cmd = chr(0x06)
    fmt = "!H"

    @classmethod
    def from_new_balance(cls, new_balance):
        if 0 <= new_balance <= 65535:
            body = struct.pack(cls.fmt, new_balance)
            return NewBalance.from_body(body)
        raise BlipperError(new_balance)

    def get_new_balance(self):
        return struct.unpack(self.fmt, self.body)[0]


@packet_type
class InsufficientFunds(Response):
    cmd = chr(0x07)
    fmt = "!H"

    @classmethod
    def from_balance(cls, balance):
        if 0 <= balance <= 65535:
            body = struct.pack(cls.fmt, balance)
            return InsufficientFunds.from_body(body)
        raise BlipperError(balance)

    def get_balance(self):
        return struct.unpack(self.fmt, self.body)[0]


@packet_type
class InvalidCardId(Response):
    cmd = chr(0x08)


@packet_type
class NextShift(Response):
    cmd = chr(0x09)

    @classmethod
    def from_text(cls, text):
        if isinstance(text, str):
            text = text.decode('utf-8')
        body = text.encode('ascii')
        return NextShift.from_body(body)


class Thread(threading.Thread):

    def __init__(self, tty=TTY, baud_rate=BAUD_RATE, logger=None, *args, **kwargs):
        if logger is None:
            self.logger = logging.getLogger('blipper.thread')
        else:
            self.logger = logger
        super(Thread, self).__init__(*args, **kwargs)
        self.tty = tty
        self.baud_rate = baud_rate
        self.ser = Serial(self.tty, self.baud_rate)
        self.ser.nonblocking()
        self.daemon = True
        self._handlers = defaultdict(list)
        self._should_exit = False
        self.waiting_for_packet = False

    def on(self, packet_type, handler):
        """Register a handler for the given packet type.

        If there are multiple handlers for a packet type, at most one
        should return a response. If no handler returns a response, a
        default OK response will be sent back.
        """
        self._handlers[packet_type].append(handler)

    def off(self, packet_type, handler):
        packet_handlers = self._handlers.get(packet_type, [])
        new_handlers = [h for h in packet_handlers if h != handler]
        self._handlers[packet_type] = new_handlers

    def send(self, packet):
        self.ser.write(bytes(packet))
        self.logger.debug('sent %r', packet)

    def get_packet(self):
        self.waiting_for_packet = True
        header = unpack_header(self.ser.read(Header.length))
        body = self.ser.read(header.body_length)
        cs = self.ser.read(1)
        self.waiting_for_packet = False

        cs_cmp = get_checksum(body)
        if cs != cs_cmp:
            raise PacketError("invalid body checksum")
        packet = packet_types[header.cmd].from_body(body)
        self.logger.debug('got %r', packet)
        return packet

    def run(self):
        while True:
            try:
                packet = self.get_packet()
                response = None
                packet_handlers = self._handlers.get(type(packet), [])
                for handler in packet_handlers:
                    handler_response = handler(packet)
                    if handler_response:
                        if response:
                            raise MultipleResponses
                        else:
                            response = handler_response
                if response is None:
                    response = packet.default_response()
                if response is not None:
                    self.ser.write(bytes(response))
            except Exception as e:
                if self._should_exit: # exception expected
                    break
                else:
                    raise e

    def join(self, timeout=None):
        self._should_exit = True
        self.ser.close()
        super(Thread, self).join(timeout)


def main():
    thread = Thread()
    try:
        thread.run()
    except KeyboardInterrupt:
        sys.exit()


if __name__ == '__main__':
    main()
