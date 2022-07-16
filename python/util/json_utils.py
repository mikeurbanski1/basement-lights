# import json
#
# import jsonpickle


# def serialize(obj):
#     return jsonpickle.encode(obj, indent=2)
# from ask_sdk_model import RequestEnvelope, Context
#
#
# def serialize(request: RequestEnvelope) -> str
#     pass
#
#
# # extend the json.JSONEncoder class
# class JSONEncoder(json.JSONEncoder):
#
#     # overload method default
#     def default(self, obj):
#
#         if isinstance(obj, RequestEnvelope):
#             converted = {
#                 'version': obj.version,
#                 'session': json.JSONEncoder.default(self, obj.session),
#                 'context': json.JSONEncoder.default(self, obj.context),
#                 'request': json.JSONEncoder.default(self, obj.request)
#             }
#         elif isinstance(obj, Context)
#
#         # Match all the types you want to handle in your converter
#         if isinstance(obj, datetime):
#             return arrow.get(obj).isoformat()
#         # Call the default method for other types
#         return json.JSONEncoder.default(self, obj)