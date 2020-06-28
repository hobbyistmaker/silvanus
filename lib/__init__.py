import logging
from logging import NullHandler as _NullHandler

logging.getLogger('silvanus.lib').addHandler(_NullHandler())
