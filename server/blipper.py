import sys
import struct

from serial import Serial

TTY = '/dev/master'
BAUD_RATE = 9600
PREAMBLE = chr(0xaa) + chr(0x55)


class PacketError(Exception):
    pass


packet_types = {}

def packet_type(cls):
    packet_types[cls.cmd] = cls
    return cls


def get_checksum(buf):
    checksum = 0x00
    for byte in buf:
        checksum += ord(byte)
    return chr(checksum % 0xff)


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
        return cls()

    def __str__(self):
        buf = PREAMBLE + self.cmd
        buf += struct.pack("!H", len(self.body))
        buf += get_checksum(buf)
        buf += self.body
        buf += get_checksum(self.body)
        return buf


class Request(Packet):

    def get_response(self):
        raise NotImplementedError()


class Response(Packet):

    def get_response(self):
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

    def get_response(self):
        return Pong()


@packet_type
class Pong(Response):
    cmd = chr(0x03)


def get_packet(ser):
    header = unpack_header(ser.read(Header.length))
    body = ser.read(header.body_length)
    cs = ser.read(1)
    cs_cmp = get_checksum(body)
    if cs != cs_cmp:
        raise PacketError("invalid body checksum")
    return packet_types[header.cmd].from_body(body)


def main():
    try:
        ser = Serial(TTY, BAUD_RATE)
        packet = get_packet(ser)
        response = packet.get_response()
        if response:
            ser.write(bytes(response))
    except KeyboardInterrupt:
        ser.close()
        sys.exit()


if __name__ == '__main__':
    main()
