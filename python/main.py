#!/usr/bin/env python3
import sys

import serial
import time

if __name__ == '__main__':

    if len(sys.argv) < 2:
        print('Error: missing serial port arg', file=sys.stderr)
        exit(1)

    port = sys.argv[1]

    ser = serial.Serial(port, 9600, timeout=1)
    ser.flush()

    print('Press enter to initialize')
    sys.stdin.readline()
    print('Sending initialization message')
    ser.write(b'Init\n')
    ser.flush()

    time.sleep(0.1)
    print(f'Response: "{ser.readline().decode("utf-8").rstrip()}"')

    while True:
        cmd = input('Enter command: ')
        print(f'Sending command: "{cmd}"')
        ser.write((cmd + '\n').encode('utf-8'))
        ser.flush()
        # time.sleep(0.1)
        # print(f'Response: "{ser.readline().decode("utf-8").rstrip()}"')

