import logging

from ask_sdk_core.dispatch_components import AbstractRequestHandler
from ask_sdk_core.utils import is_intent_name

from python.alexa import sb
from python.alexa.commands import send_command
from python.alexa.handlers.phrases import NOT_CONNECTED
from python.alexa.handlers.utils import get_intent_value

logger = logging.getLogger('flask_ask')


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


class ColorIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return is_intent_name("ColorIntent")(handler_input)

    def handle(self, handler_input):
        logger.debug('Handling ColorIntent')

        color = get_intent_value(handler_input.request_envelope.request, 'color').upper()
        logger.debug(f'Color: {color}')

        if color in colors:
            success = send_command(color)
            response = 'ok' if success else NOT_CONNECTED
        else:
            response = f'I did not recognize the color {color}'

        handler_input.response_builder.speak(response)
        return handler_input.response_builder.response


class ComponentColorChangeIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return is_intent_name("ComponentColorChangeIntent")(handler_input)

    def handle(self, handler_input):
        logger.debug('Handling ComponentColorChangeIntent')

        values = get_intent_value(handler_input.request_envelope.request, ['color', 'value'])
        color = values.get('color')
        value = values.get('value')

        if color.lower() not in ['red', 'green', 'blue']:
            response = 'Color must be red, green, or blue'
        else:
            value = int(value)
            if value < 0:
                value = 0
            elif value > 255:
                value = 255

            if send_command(f'SET_COLOR {color.upper()} {value}'):
                response = 'ok'
            else:
                response = NOT_CONNECTED

        handler_input.response_builder.speak(response)
        return handler_input.response_builder.response


class ListColorsIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return is_intent_name("ListColorsIntent")(handler_input)

    def handle(self, handler_input):
        logger.debug('Handling ListColorsIntent')

        handler_input.response_builder.speak(f'Here are the colors: {", ".join(colors + ["rainbow"])}')
        return handler_input.response_builder.response


sb.add_request_handler(ColorIntentHandler())
sb.add_request_handler(ListColorsIntentHandler())
sb.add_request_handler(ComponentColorChangeIntentHandler())
