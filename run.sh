#!/bin/bash
gunicorn --bind unix:alexa.sock -m 777 python.wsgi:app -e COM_PORT="$(ls /dev/ttyACM* || echo x)" -e LOG_DIR="$(pwd)/logs"
