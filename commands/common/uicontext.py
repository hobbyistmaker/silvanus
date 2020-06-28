import traceback

from contextlib import contextmanager

# noinspection SpellCheckingInspection
# noinspection PyBroadException
@contextmanager
def uicontext(app):
    try:
        yield
    except:
        trace = traceback.format_exc()
        app.ui.messageBox(f'{trace}')
