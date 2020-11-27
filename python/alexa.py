import logging.handlers
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

handler = logging.handlers.RotatingFileHandler('logs/alexa.log', maxBytes=1024 * 10, backupCount=5)
handler.setFormatter(logging.Formatter('%(asctime)s - %(levelname)s - %(message)s'))
logger.addHandler(handler)

mode_names = [
    'progressive rainbow',
    'progressive solid',
    'progressive rainbow fast',
    'progressive solid fast',
    'starry night',
    'breathing',
    'breathing rainbow',
    'breathing solid'
]

mode_mapping = {}
for mode in mode_names:
    mode_mapping[mode] = f'MODE_{len(mode_mapping)}' # MODE_0, MODE_1, ...
mode_mapping['cycle'] = 'AUTOCYCLE_ON'

colors = [
  'RED',
  'BLUE',
  'GREEN',
  'WHITE',
  'PINK',
  'YELLOW',
  'PURPLE',
  'CYAN',
  'ORANGE'
]

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


port_monitor = threading.Thread(target=port_monitor, name='PortMonitorThread', daemon=True)
port_monitor.start()


def shutdown():
    logger.warning('Shutting down in 5 seconds')
    time.sleep(5)
    send_command(status_mapping['initialized'])
    os.system('sudo shutdown now')


@app.route('/hello')
def hello_world():
    return 'hello\n'


@app.route('/startup/<status>', methods=['POST'])
def set_startup_status(status):

    command = status_mapping.get(status)

    if not command:
        return f'Invalid startup status: {status}'

    if send_command(command):
        return f'Sent command: {command}\n'
    else:
        return 'Arduino was not connected\n'


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
    elif status.lower() == 'rainbow':
        ret = send_command('RAINBOW')
    else:
        return statement(f'I did not recognize the status {status}')

    if ret:
        # return statement(f'I set the lights to {status}')
        return statement('ok')
    else:
        return statement('It seems that Arduino is not connected')


@ask.intent('ShutdownIntent')
def set_lights(room):
    threading.Thread(target=shutdown, name='ShutdownThread').start()
    return statement('ok')


@ask.intent('ModeIntent', mapping={'mode': 'mode'})
def set_mode(mode, room):
    logger.info(mode)
    mode_command = mode_mapping.get(mode.lower())
    if mode_command:
        if send_command(mode_command):
            # return statement(f'I set the light mode to {mode}')
            return statement('ok')
        else:
            return statement('It seems that Arduino is not connected')
    else:
        return statement(f'I did not recognize the mode named {mode}')


@ask.intent('ColorIntent', mapping={'color': 'color'})
def set_color(color, room):
    logger.info(color)
    if send_command(color.upper()):
        # return statement(f'I set the color to {color}')
        return statement('ok')
    else:
        return statement('It seems that Arduino is not connected')


@ask.intent('ListColorsIntent')
def list_colors(room):
    return statement(f'Here are the colors: {", ".join(colors + ["rainbow"])}')


@ask.intent('ListModesIntent')
def list_modes(room):
    return statement(f'Here are the modes: {", ".join(mode_mapping.keys())}')


@ask.intent('BrightnessIntent')
def list_modes(direction, brightness, room):
    if direction:
        ret = send_command(f'{direction.upper()}_BRIGHTNESS')
    else:
        brightness = int(brightness)
        if brightness < 1:
            brightness = 1
        elif brightness > 255:
            brightness = 255
        ret = send_command(f'BRIGHTNESS {brightness}')

    if ret:
        return statement('ok')
    else:
        return statement('It seems that Arduino is not connected')


@ask.intent('AMAZON.HelpIntent')
def help():
    speech_text = 'You can say: turn the light on, off, solid, or rainbow; set the mode to blank; ' \
                  'change the color to blank; set the brightness to blank, or increase or decrease the ' \
                  'brightness. You can also ask for a list of modes or colors.'
    return question(speech_text).reprompt(speech_text)


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
    if 'ASK_VERIFY_REQUESTS' in os.environ:
        verify = str(os.environ.get('ASK_VERIFY_REQUESTS', '')).lower()
        if verify == 'false':
            app.config['ASK_VERIFY_REQUESTS'] = False
    app.run(host='0.0.0.0', port=7626, ssl_context=('cert.pem', 'key.pem'))



