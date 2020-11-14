#!/usr/bin/env python3
import sys

import serial
import time

if __name__ == '__main__':
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
    ser.flush()

    ser.write(b'Hello\n')
    print(ser.readline().decode('utf-8').rstrip())

    for line in sys.stdin:
        ser.write((line.rstrip() + '\n').encode('utf-8'))
        print(ser.readline().decode('utf-8').rstrip())
        ser.flush()
        print(ser.readline().decode('utf-8').rstrip())
        ser.flush()
        time.sleep(0.5)
