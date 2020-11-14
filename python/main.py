#!/usr/bin/env python3
import sys

import serial
import time

if __name__ == '__main__':

    # if len(sys.argv) < 2:
    #     print('Error: missing serial port arg', file=sys.stderr)
    #     exit(1)
    #
    # port = sys.argv[1]
    #
    # ser = serial.Serial(port, 9600, timeout=1)
    # ser.flush()
    #
    # print('Press enter')
    # sys.stdin.readline()
    # print('Sending initialization message')
    # ser.write(b'Init\n')
    # ser.flush()

    while True:
        cmd = input('Enter command: ')
        print("'" + cmd + "'")

    # for line in sys.stdin:
    #     ser.write((line.rstrip() + '\n').encode('utf-8'))
    #     print(ser.readline().decode('utf-8').rstrip())
    #     ser.flush()
    #     print(ser.readline().decode('utf-8').rstrip())
    #     ser.flush()
    #     time.sleep(0.5)
