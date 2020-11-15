import logging
import os
import serial
import time
from flask import Flask
from flask_ask import Ask, request, session, question, statement

app = Flask(__name__)
ask = Ask(app, "/")
logging.getLogger('flask_ask').setLevel(logging.DEBUG)

logger = logging.getLogger('flask_ask')

port = os.environ.get('COM_PORT', '/dev/ttyACM0')

mode_mapping = {
    'binary': 'MODE_0',
    'alternating': 'MODE_1',
    'autocycle': 'AUTOCYCLE_ON'
}


@ask.launch
def launch():
    speech_text = 'Welcome to Raspberry Pi Automation.'
    return question(speech_text).reprompt(speech_text).simple_card(speech_text)


@ask.intent('LightIntent', mapping={'status': 'status'})
def set_lights(status, room):
    logger.info(status)

    if status.lower() == 'off':
        send_command('OFF')
    elif status.lower() == 'solid':
        send_command('SOLID')
    elif status.lower() == 'on':
        send_command('SOLID')
    else:
        return statement(f'I did not recognize the status {status}')

    return statement(f'I set the lights to {status}')


@ask.intent('ModeIntent', mapping={'mode': 'mode'})
def set_mode(mode, room):
    logger.info(mode)
    mode_command = mode_mapping.get(mode.lower())
    if mode_command:
        send_command(mode_command)
        return statement(f'I set the light mode to {mode}')
    else:
        return statement(f'I did not recognize the mode named {mode}')


@ask.intent('ColorIntent', mapping={'color': 'color'})
def set_color(color, room):
    logger.info(color)
    send_command(color.upper())
    return statement(f'I set the color to {color}')


@ask.intent('AMAZON.HelpIntent')
def help():
    speech_text = 'You can say hello to me!'
    return question(speech_text).reprompt(speech_text).simple_card('HelloWorld', speech_text)


@ask.session_ended
def session_ended():
    return "{}", 200


def send_command(command):
    logger.info(f'Sending command: "{command}"')
    # ser.write((command + '\n').encode('utf-8'))
    # ser.flush()


if __name__ == '__main__':

    # ser = serial.Serial(port, 9600, timeout=1)
    # ser.flush()
    #
    # logger.info(f'Sleeping for 5 seconds while arduino boots')
    # time.sleep(5)
    # print('Sending initialization message')
    # send_command('init')
    # time.sleep(0.1)
    # print(f'Response: "{ser.readline().decode("utf-8").rstrip()}"')

    if 'ASK_VERIFY_REQUESTS' in os.environ:
        verify = str(os.environ.get('ASK_VERIFY_REQUESTS', '')).lower()
        if verify == 'false':
            app.config['ASK_VERIFY_REQUESTS'] = False
    app.run(debug=True)



