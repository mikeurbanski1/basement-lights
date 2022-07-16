from typing import List, Union, Dict, Optional

from ask_sdk_model import IntentRequest, Request


def get_intent_value(request: Optional[Request], slots: Union[str, List[str]]) -> Union[str, Dict[str, str]]:

    assert isinstance(request, IntentRequest)

    values = {}
    intent_slots = request.intent.slots

    if isinstance(slots, str):
        return intent_slots[slots].value

    for slot in slots:
        values[slot] = intent_slots[slot].value
    return values
