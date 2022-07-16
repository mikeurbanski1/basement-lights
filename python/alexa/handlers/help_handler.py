import logging

from ask_sdk_core.dispatch_components import AbstractRequestHandler
from ask_sdk_core.utils import is_intent_name

from python.alexa import sb
from python.alexa.handlers.phrases import HELP

logger = logging.getLogger('flask_ask')


class HelpIntentHandler(AbstractRequestHandler):

    def can_handle(self, handler_input):
        return is_intent_name("AMAZON.HelpIntent")(handler_input)

    def handle(self, handler_input):
        logger.debug('Handling HelpIntent')
        handler_input.response_builder.speak(HELP)
        return handler_input.response_builder.response


sb.add_request_handler(HelpIntentHandler())
