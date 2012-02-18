from serial import Serial

TTY = '/dev/master'
BAUD_RATE = 9600


def main():
    ser = Serial(TTY, BAUD_RATE)
    print "port is", ser.portstr
    ser.close()


if __name__ == '__main__':
    main()
