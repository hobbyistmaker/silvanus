import traceback

import adsk.core

from contextlib import contextmanager


# noinspection PyBroadException
@contextmanager
def appcontext():
    app = adsk.core.Application.get()
    try:
        yield app
    except:
        trace = traceback.format_exc()
        app.userInterface.messageBox(f'{trace}')
