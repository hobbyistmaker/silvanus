import json
import os

from types import SimpleNamespace


script_dir = os.path.dirname(__file__)
file_path = os.path.join(script_dir, 'config.json')

def dict_to_sns(d):
    return SimpleNamespace(**d)


with open(file_path, 'r') as js:
    ns_config = json.load(js, object_hook=dict_to_sns)

with open(file_path, 'r') as js:
    config = json.load(js)


inputs = ns_config.inputs
ids = ns_config.command_ids
input_defaults = ns_config.input_defaults
