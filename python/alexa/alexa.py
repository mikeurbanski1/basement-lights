import logging
import threading
from logging import handlers, DEBUG

from flask import Flask
from flask_ask_sdk.skill_adapter import SkillAdapter

import os

from python.alexa import sb
from python.alexa.commands import status_mapping, send_command, port_monitor
from python.alexa.handlers import *

logger = logging.getLogger('flask_ask')
formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')

handler = logging.handlers.RotatingFileHandler('logs/alexa.log', maxBytes=1024 * 10, backupCount=5)
handler.setFormatter(formatter)
logger.addHandler(handler)

handler = logging.StreamHandler()
handler.setFormatter(formatter)
logger.addHandler(handler)

logger.setLevel(DEBUG)

app = Flask(__name__)
skill_response = SkillAdapter(skill=sb.create(), skill_id='amzn1.ask.skill.15dc0b67-8d15-4262-a35d-8333ae4568f0', app=app)
skill_response.register(app=app, route="/")

port_monitor = threading.Thread(target=port_monitor, name='PortMonitorThread', daemon=True)
port_monitor.start()


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


if __name__ == '__main__':
    if 'ASK_VERIFY_REQUESTS' in os.environ:
        verify = str(os.environ.get('ASK_VERIFY_REQUESTS', '')).lower()
        if verify == 'false':
            app.config['ASK_VERIFY_REQUESTS'] = False
    app.run(host='0.0.0.0', port=7626) #, ssl_context=('cert.pem', 'key.pem'))
