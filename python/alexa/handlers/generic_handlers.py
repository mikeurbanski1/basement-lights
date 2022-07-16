import logging

from ask_sdk_core.dispatch_components import AbstractRequestHandler, AbstractExceptionHandler
from ask_sdk_core.utils import is_request_type, is_intent_name
from ask_sdk_model.ui import SimpleCard

from python.alexa import sb
from python.alexa.handlers.phrases import CANCEL, FALLBACK, LAUNCH

logger = logging.getLogger('flask_ask')


class LaunchRequestHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return is_request_type("LaunchRequest")(handler_input)

    def handle(self, handler_input):
        logger.debug('Handling LaunchRequest')
        speech_text = LAUNCH
        card = SimpleCard(LAUNCH, speech_text)
        return handler_input.response_builder.speak(speech_text).set_card(card).response


class SessionEndedRequestHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return is_request_type('SessionEndedRequest')(handler_input)

    def handle(self, handler_input):
        logger.debug('Handling SessionEndedRequest')
        logger.debug('Handling SessionEndedRequest')
        return handler_input.response_builder.response


class FallbackIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return is_intent_name("AMAZON.FallbackIntent")(handler_input)

    def handle(self, handler_input):
        logger.debug('Handling FallbackIntent')
        handler_input.response_builder.speak(FALLBACK)
        return handler_input.response_builder.response


class CancelOrStopIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return (is_intent_name("AMAZON.CancelIntent")(handler_input) or
                is_intent_name("AMAZON.StopIntent")(handler_input))

    def handle(self, handler_input):
        logger.debug('Handling CancelIntent or StopIntent')
        handler_input.response_builder.speak(CANCEL)
        return handler_input.response_builder.response


class CatchAllExceptionHandler(AbstractExceptionHandler):
    def can_handle(self, handler_input, exception):
        return True

    def handle(self, handler_input, exception):
        logger.error(f'Encountered following exception: {exception}')

        speech = 'There was an error handling the request'
        handler_input.response_builder.speak(speech).ask(speech)
        return handler_input.response_builder.response


sb.add_request_handler(LaunchRequestHandler())
sb.add_request_handler(SessionEndedRequestHandler())
sb.add_request_handler(FallbackIntentHandler())
sb.add_request_handler(CancelOrStopIntentHandler())
sb.add_exception_handler(CatchAllExceptionHandler())
