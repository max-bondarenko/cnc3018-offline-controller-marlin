import random
import sys
import termios
import threading
import tty
from collections import deque

N = -1
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
        resp.append("!!: _123456789_12345678_==\r\n")


def line_number(str):
    if str.startswith("N"):
        global N
        N += 1
        if N == 55:
            resp.append("Error: Resend Last Line:123\nResend:50\nok\n")
            # stack.append(expect_line)
            return
        if N == 50:
            resp.append("Error: Resend Last Line:123\nResend:49\nok\n")

            return
        else:
            global x
            global P
            T = random.randint(10, 250)
            P = random.randint(1, 128)

            x += 1.03
            resp.append(
                "ok C: X:{0:2.2f} Y:{1:2.2f} Z:{2:2.2f} E:{3:2.2f} T:{4} /123 @:{5} B:103\n".format(x, y, z, N, T, P))
    else:
        resp.append("ok\r\n")
    stack.append(line_number)


def marlin_repeater(str):
    global x
    global P
    if str.startswith("G0") or str.startswith("M114"):
        x = x + 0.01
        resp.append("ok C: X:{0:2.2f} Y:{1:2.2f} Z:{2:2.2f} E:{3:2.2f} @:{4} B:{4} @:{4}\r\n".format(x, y, z, e, P))
        stack.append(marlin_repeater)
    if str.startswith("G28"):
        P = 256
        resp.append("ok C: X:{0:2.2f} Y:{1:2.2f} Z:{2:2.2f} E:{3:2.2f} @:{4} B:{4} @:127\r\n".format(x, y, z, e, P))
        stack.append(marlin_repeater)
    elif str.startswith("M104"):
        P = P << 1
        if P > 256:
            P = 1
        resp.append("ok C: X:{0:2.2f} Y:{1:2.2f} Z:{2:2.2f} E:{3:2.2f} @:{4} B@:{4}\r\n".format(x, y, z, e, P))
        stack.append(marlin_repeater)
    elif str.startswith("G92"):
        resp.append("echo:busy\n")
    elif str.startswith("G91"):
        resp.append("ok\n")
    elif str.startswith("M110N0"):
        resp.append("ok\r\n")

        global N
        N = 0
        stack.append(line_number)
    elif str.startswith("M302"):
        resp.append("echo: min temp 150C\n")
        stack.append(marlin_repeater)
    elif str.startswith("M"):
        resp.append("ok\n")
        stack.append(marlin_repeater)


def last_state(str):
    if str.startswith("M115"):
        resp.append("FIRMWARE_NAME:Marlin\r\n"
                    "Cap:AUTOREPORT_POS:1\r\n"
                    "Cap:AUTOREPORT_TEMP:1\r\n"
                    "Cap:EMERGENCY_PARSER:1\r\n")
        stack.append(marlin_repeater)
    elif str.startswith("N"):
        line_number(str)
    else:
        marlin_repeater(str)


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

            attr[tty.ISPEED] = termios.B115200
            attr[tty.OSPEED] = termios.B115200
            termios.tcsetattr(fd, termios.TCSADRAIN, attr)
            print('>>o_flag:{:b}'.format(attr[tty.OFLAG]))
            print(
                '{0:b}\n{1:b}\n{2:b}\n{3:b}'.format(attr[tty.IFLAG], attr[tty.OFLAG], attr[tty.CFLAG], attr[tty.LFLAG]))
            a = ""

            fw.writelines(["ok\n"])
            stack.append(last_state)
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
                        pop = stack[stack.__len__() - 1]
                        pop(a)
                        if resp.__len__() > 0:
                            resp_pop = resp.pop()
                            if stack.__len__() > 1:
                                stack.pop()  # do not remove last state
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
