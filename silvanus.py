# Author-pahobbyistmaker@gmail.com
# Description-Finger-tabbed box generator for Fusion 360

# Icons made by <a href="https://www.flaticon.com/authors/freepik" title="Freepik">Freepik</a> from <a href="https://www.flaticon.com/" title="Flaticon"> www.flaticon.com</a>

from .commands.basicboxcommand import BasicBoxCommand
from .util.appcontext import appcontext

APPLICATION_STARTUP = 'IsApplicationStartup'
STARTUP_MSG = 'Silvanus Module Started Successfully.'

commands = []


def run(context):
    with appcontext() as app:
        basic = BasicBoxCommand(app)
        commands.append(basic)
        basic.start()

        if context[APPLICATION_STARTUP] is False:
            app.userInterface.messageBox(STARTUP_MSG)


def stop(context):
     with appcontext():
        for command in commands:
            command.stop()
