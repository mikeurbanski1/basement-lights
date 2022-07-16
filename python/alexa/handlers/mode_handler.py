import logging

from ask_sdk_core.dispatch_components import AbstractRequestHandler
from ask_sdk_core.utils import is_intent_name

from python.alexa import sb
from python.alexa.commands import send_command
from python.alexa.handlers.phrases import NOT_CONNECTED
from python.alexa.handlers.utils import get_intent_value

logger = logging.getLogger('flask_ask')


mode_names = [
    'progressive rainbow',
    'progressive solid',
    'progressive rainbow fast',
    'progressive solid fast',
    'starry night',
    'breathing',
    'breathing rainbow',
    'breathing solid',
    'zip',
    'random zip',
    'partial rainbow',
    'partial rainbow fast'
]

mode_mapping = {}
for mode in mode_names:
    mode_mapping[mode] = f'MODE_{len(mode_mapping)}' # MODE_0, MODE_1, ...
mode_mapping['cycle'] = 'AUTOCYCLE_ON'


class ModeIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return is_intent_name("ModeIntent")(handler_input)

    def handle(self, handler_input):
        logger.debug('Handling ModeIntent')

        mode_value = get_intent_value(handler_input.request_envelope.request, 'mode').lower()
        logger.debug(f'Mode: {mode_value}')

        if mode_value in mode_mapping:
            mode = mode_mapping[mode_value]
            success = send_command(mode)
            response = 'ok' if success else NOT_CONNECTED
        else:
            response = f'I did not recognize the mode named {mode_value}'

        handler_input.response_builder.speak(response)
        return handler_input.response_builder.response


class ListModesIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return is_intent_name("ListModesIntent")(handler_input)

    def handle(self, handler_input):
        logger.debug('Handling ListModesIntent')

        handler_input.response_builder.speak(f'Here are the modes: {", ".join(mode_mapping.keys())}')
        return handler_input.response_builder.response


sb.add_request_handler(ModeIntentHandler())
sb.add_request_handler(ListModesIntentHandler())
