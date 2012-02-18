import sys
import struct

from serial import Serial

TTY = '/dev/master'
BAUD_RATE = 9600
PREAMBLE = chr(0xaa) + chr(0x55)
HEADER_FORMAT = "!BBH" # network/big-endian, unsigned char, unsigned char
PAYLOAD_LENGTH_FORMAT = "!H" # network/big-endian, unsigned short
HEAD_LENGTH = len(PREAMBLE) + 2 + 1


class PacketError(Exception):
    pass


def get_checksum(buf):
    checksum = 0x00
    for byte in buf:
        checksum += ord(byte)
    return chr(checksum % 0xff)



def pack_header(payload):
    return struct.pack(PAYLOAD_LENGTH_FORMAT, len(payload))


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
    cmd = fields[2]
    length = fields[3]
    cs = chr(fields[4])
    cs_cmp = get_checksum(buf[:5])
    if cs != cs_cmp:
        raise PacketError("%s != %s" % (cs, cs_cmp))
    return Header(cmd, length)


class Command(object):

    def __init__(self, payload=None):
        self.payload = payload or ''

    def __str__(self):
        buf = PREAMBLE + self.cmd
        buf += struct.pack("!H", len(self.payload))
        buf += get_checksum(buf)
        buf += self.payload
        buf += get_checksum(self.payload)
        return buf


class RequestCommand(Command):
    pass


class ResponseCommand(Command):
    pass


class OkCommand(RequestCommand):
    cmd = chr(0x00)


class FailCommand(ResponseCommand):
    cmd = chr(0x01)


class PingCommand(RequestCommand):
    cmd = chr(0x02)


class PongCommand(ResponseCommand):
    cmd = chr(0x03)


def cycle(ser):
    header = unpack_header(ser.read(Header.length))
    body = ser.read(header.body_length)
    print header, body


def main():
    try:
        ser = Serial(TTY, BAUD_RATE)
        cycle(ser)
    except KeyboardInterrupt:
        ser.close()
        sys.exit()


if __name__ == '__main__':
    main()
