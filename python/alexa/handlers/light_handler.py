import logging

from ask_sdk_core.dispatch_components import AbstractRequestHandler
from ask_sdk_core.utils import is_intent_name

from python.alexa import sb
from python.alexa.commands import send_command
from python.alexa.handlers.phrases import NOT_CONNECTED
from python.alexa.handlers.utils import get_intent_value

logger = logging.getLogger('flask_ask')


status_mapping = {
    'off': 'OFF',
    'solid': 'SOLID',
    'on': 'SOLID',
    'rainbow': 'RAINBOW'
}


class LightIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return is_intent_name("LightIntent")(handler_input)

    def handle(self, handler_input):
        logger.debug('Handling LightIntent')

        status = get_intent_value(handler_input.request_envelope.request, 'status').lower()
        logger.debug(f'Status: {status}')

        if status in status_mapping:
            mode = status_mapping[status]
            success = send_command(mode)
            response = 'ok' if success else NOT_CONNECTED
        else:
            response = f'I did not recognize the status {status}'

        handler_input.response_builder.speak(response)
        return handler_input.response_builder.response


sb.add_request_handler(LightIntentHandler())
