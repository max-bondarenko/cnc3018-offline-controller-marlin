import sys
import termios
import threading
import tty
from collections import deque

N = 0
x: float = 0.0
y: float = 0.0
z: float = 10.01
e: float = 0.01
P: int = 1

stack = deque()
resp = deque()


def expect_line(str):
    if str.startswith("N3"):
        resp.append("ok\r\n")
        stack.append(line_number)
    else:
        resp.append("!!: Wrong number\r\n")


def line_number(str):
    if str.startswith("N"):
        global N
        N += 1
        print("\t\t N++")
        if N == 3:
            resp.append("Resend:3\r\n")
            stack.append(expect_line)
            return
        if N == 5:
            resp.append("Error: some text\r\n")
            return
        else:
            global x
            x += 1.03
            resp.append("ok C: X:{0:2.2f} Y:{1:2.2f} Z:{2:2.2f} E:{3:2.2f}\r\n".format(x, y, z, e))
    stack.append(line_number)


def marlin_repeater(str):
    global x
    global P
    if str.startswith("G0") or str.startswith("M114"):
        x = x + 0.01
        resp.append("ok C: X:{0:2.2f} Y:{1:2.2f} Z:{2:2.2f} E:{3:2.2f} @:{4} B:{4} @:{4}\r\n".format(x, y, z, e, P))
    elif str.startswith("M104"):
        P = P << 1
        if P > 256:
            P = 1
        resp.append("ok C: X:{0:2.2f} Y:{1:2.2f} Z:{2:2.2f} E:{3:2.2f} @:{4} B@:{4}\r\n".format(x, y, z, e, P))
    # comment this branch to check power
    # elif str.startswith("M110N0"):
    #     resp.append("ok\r\n")
    #     global N
    #     N = 0
    #     stack.append(line_number)
    #     return
    else:
        resp.append("ok\r\n")

    stack.append(marlin_repeater)


def check_mode(str):
    if str.startswith("M115"):
        resp.append("FIRMWARE_NAME:Marlin\r\n"
                    "Cap:AUTOREPORT_POS:1\r\n"
                    "Cap:AUTOREPORT_TEMP:1\r\n"
                    "Cap:EMERGENCY_PARSER:1\r\n")
        stack.append(marlin_repeater)
    else:
        stack.append(check_mode)


def writeOkEach2sec(fw):
    def foo():
        print("0k")
        if not fw.closed:
            fw.write("ok C: X:{0:6.2f} Y:{1:6.2f} Z:{2: 6.2f} E:{3: 6.2f}\r\n".format(x, y, z, e))
        threading.Timer(2, foo).start()

    foo()


def write_ok(fw):
    fw.write("ok C: X:{0:6.2f} Y:{1:6.2f} Z:{2: 6.2f} E:{3: 6.2f}\r\n".format(x, y, z, e))


def main():
    a = '/dev/ttyUSB0'

    q = []

    def check(a):
        if a == 'I':
            return "[VER:1.0"

    q.append(check)

    fw = open(a, 'w', encoding='ascii')

    try:
        with open(a, 'rb') as f:
            fd = f.fileno()
            attr = termios.tcgetattr(fd)

            termios.tcdrain(fd)
            print('o_flag:{:b}'.format(attr[tty.OFLAG]))
            attr[tty.IFLAG] = attr[tty.IFLAG] & ~termios.ISTRIP

            attr[tty.IFLAG] = attr[tty.IFLAG] | termios.IXOFF
            attr[tty.IFLAG] = attr[tty.IFLAG] | termios.IXON
            attr[tty.IFLAG] = attr[tty.IFLAG] | termios.CSTOPB
            attr[tty.IFLAG] = attr[tty.IFLAG] | termios.IGNCR
            #  ==================================================
            attr[tty.OFLAG] = attr[tty.OFLAG] | termios.CS8
            attr[tty.OFLAG] = attr[tty.OFLAG] & ~termios.CSTOPB

            #  ==================================================
            attr[tty.CFLAG] = attr[tty.CFLAG] | termios.CLOCAL

            attr[tty.ISPEED] = termios.B9600
            attr[tty.OSPEED] = termios.B9600
            termios.tcsetattr(fd, termios.TCSADRAIN, attr)
            print('>>o_flag:{:b}'.format(attr[tty.OFLAG]))
            print(
                '{0:b}\n{1:b}\n{2:b}\n{3:b}'.format(attr[tty.IFLAG], attr[tty.OFLAG], attr[tty.CFLAG], attr[tty.LFLAG]))
            a = ""

            stack.append(check_mode)
            while f.readable():
                b = f.read(1)
                print(".", end='')

                if b != b'\r':
                    if b.isalnum():
                        a += b.decode(encoding='utf-8')
                else:
                    print('')

                    while stack.__len__() > 0:
                        print("> " + a)
                        pop = stack.pop()
                        pop(a)
                        if resp.__len__() > 0:
                            resp_pop = resp.pop()
                            print("<  " + resp_pop)
                            fw.writelines([resp_pop])
                        break

                    a = ""
    except KeyboardInterrupt:
        fw.close()
        f.close()

    return 0


if __name__ == '__main__':
    # print("{0: 6.2f}".format(x))
    sys.exit(main())
