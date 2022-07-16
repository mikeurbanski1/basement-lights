import logging
import os
import time

import serial

port = None
ser = None

logger = logging.getLogger('flask_ask')

status_mapping = {
    'ngrok_running': 'INIT_0',
    'manifest_retrieved': 'INIT_1',
    'manifest_updated': 'INIT_2',
    'initialized': 'INIT_FINAL'
}


def setup_serial():
    global port
    global ser
    ports = [a for a in os.listdir('/dev') if 'ttyACM' in a]
    if ports:
        try_port = os.path.join('/dev/', ports[0])
        if try_port != port:
            logger.info(f'Setting port to {try_port}')
            port = try_port

            ser = serial.Serial(port, 9600, timeout=1)
            ser.flush()
    else:
        if port:
            logger.info('Setting port to None')
            port = None


def port_monitor():
    logger.info('Started port monitor thread')
    while True:
        setup_serial()
        time.sleep(5)


def shutdown():
    logger.warning('Shutting down in 5 seconds')
    time.sleep(5)
    send_command(status_mapping['initialized'])
    os.system('sudo shutdown now')


def send_command(command):
    logger.info(f'Sending command: "{command}"')
    return True
    if not port:
        logger.error('Serial is not connected')
        return False
    else:
        ser.write((command + '\n').encode('utf-8'))
        ser.flush()
        return True
