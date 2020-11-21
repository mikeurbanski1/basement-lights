import logging
import os
import serial
import time
from flask import Flask
from flask_ask import Ask, request, session, question, statement

import threading

port = None
ser = None

app = Flask(__name__)
ask = Ask(app, "/")
logging.getLogger('flask_ask').setLevel(logging.DEBUG)

logger = logging.getLogger('flask_ask')

mode_mapping = {
    'binary': 'MODE_0',
    'alternating': 'MODE_1',
    'autocycle': 'AUTOCYCLE_ON'
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

            logger.info(f'Sleeping for 5 seconds while arduino boots')
            time.sleep(5)
            # print('Sending initialization message')
            # send_command('init')
            # time.sleep(0.1)
            # print(f'Response: "{ser.readline().decode("utf-8").rstrip()}"')
    else:
        if port:
            logger.info('Setting port to None')
            port = None


def port_monitor():
    logger.info('Started port monitor thread')
    while True:
        setup_serial()
        time.sleep(5)


port_monitor = threading.Thread(target=port_monitor(), name='PortMonitorThread', daemon=True)


@app.route('/hello')
def hello_world():
    logger.info(port)
    return 'Hello World!'


@ask.launch
def launch():
    speech_text = 'Welcome to Raspberry Pi Automation.'
    return question(speech_text).reprompt(speech_text).simple_card(speech_text)


@ask.intent('LightIntent', mapping={'status': 'status'})
def set_lights(status, room):
    logger.info(status)

    if status.lower() == 'off':
        ret = send_command('OFF')
    elif status.lower() == 'solid':
        ret = send_command('SOLID')
    elif status.lower() == 'on':
        ret = send_command('SOLID')
    else:
        return statement(f'I did not recognize the status {status}')

    if ret:
        return statement(f'I set the lights to {status}')
    else:
        return statement('It seems that Arduino is not connected')


@ask.intent('ModeIntent', mapping={'mode': 'mode'})
def set_mode(mode, room):
    logger.info(mode)
    mode_command = mode_mapping.get(mode.lower())
    if mode_command:
        if send_command(mode_command):
            return statement(f'I set the light mode to {mode}')
        else:
            return statement('It seems that Arduino is not connected')
    else:
        return statement(f'I did not recognize the mode named {mode}')


@ask.intent('ColorIntent', mapping={'color': 'color'})
def set_color(color, room):
    logger.info(color)
    if send_command(color.upper()):
        return statement(f'I set the color to {color}')
    else:
        return statement('It seems that Arduino is not connected')


@ask.intent('AMAZON.HelpIntent')
def help():
    speech_text = 'You can say hello to me!'
    return question(speech_text).reprompt(speech_text).simple_card('HelloWorld', speech_text)


@ask.session_ended
def session_ended():
    return "{}", 200


def send_command(command):
    logger.info(f'Sending command: "{command}"')
    if not port:
        logger.error('Serial is not connected')
        return False
    else:
        ser.write((command + '\n').encode('utf-8'))
        ser.flush()
        return True


if __name__ == '__main__':
    logger.debug('debug')
    logger.info('info')
    logger.warning('warning')
    logger.error('error')

    if 'ASK_VERIFY_REQUESTS' in os.environ:
        verify = str(os.environ.get('ASK_VERIFY_REQUESTS', '')).lower()
        if verify == 'false':
            app.config['ASK_VERIFY_REQUESTS'] = False
    app.run(host='0.0.0.0', port=7626, ssl_context=('cert.pem', 'key.pem'))



