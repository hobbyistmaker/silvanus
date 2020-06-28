import logging

from logging import NullHandler as _NullHandler

from .log import config_log
from .appcontext import appcontext

logging.getLogger('silvanus').getChild(__name__).addHandler(_NullHandler())
