import logging
import traceback

from contextlib import contextmanager


# noinspection SpellCheckingInspection
# noinspection PyBroadException
@contextmanager
def uicontext(app, level=''):
    logger = logging.getLogger('silvanus').getChild(level)

    try:
        yield
    except:
        trace = traceback.format_exc()
        logger.error(f'{level}: {trace}')
        app.userInterface.messageBox(f'{trace}')
