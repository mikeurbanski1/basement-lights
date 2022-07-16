import logging

from ask_sdk_core.dispatch_components import AbstractRequestHandler
from ask_sdk_core.utils import is_intent_name

from python.alexa import sb
from python.alexa.commands import send_command
from python.alexa.handlers.phrases import NOT_CONNECTED
from python.alexa.handlers.utils import get_intent_value

logger = logging.getLogger('flask_ask')


class BrightnessIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return is_intent_name("BrightnessIntent")(handler_input)

    def handle(self, handler_input):
        logger.debug('Handling BrightnessIntent')

        values = get_intent_value(handler_input.request_envelope.request, ['direction', 'brightness'])
        direction = values.get('direction')
        brightness = values.get('brightness')

        logger.debug(f'Direction: {direction}')
        logger.debug(f'Brightness: {brightness}')

        if not direction and not brightness:
            handler_input.response_builder.speak('I did not understand - you can say increase the brightness, or set the brightness to 100')
            return handler_input.response_builder.response

        if direction:
            success = send_command(f'{direction.upper()}_BRIGHTNESS')
        else:
            brightness = int(brightness)
            if brightness < 1:
                brightness = 1
            elif brightness > 255:
                brightness = 255
            success = send_command(f'BRIGHTNESS {brightness}')

        response = 'ok' if success else NOT_CONNECTED

        handler_input.response_builder.speak(response)
        return handler_input.response_builder.response


sb.add_request_handler(BrightnessIntentHandler())
