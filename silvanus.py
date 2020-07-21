# Author-Hobbyist Maker
# Description-Finger-tabbed box generator for Fusion 360

import logging.config
import sys

import adsk.core

from .lib.generateboxcommand import GenerateBoxCommand
from .lib.common.commandbutton import CommandButton
from .util import appcontext
from .util import config_log

FILENAME = 'logs/lib.log'
MAX_LEVEL = 'DEBUG'

logger = logging.getLogger('silvanus')

log_file = config_log(__path__, MAX_LEVEL, FILENAME)
logging.config.dictConfig(log_file)

logger.info('Starting Silvanus plugin.')
logger.debug(f'Python version: {sys.version}')
logger.debug(f'Fusion 360 version: {adsk.core.Application.get().version}')


APPLICATION_STARTUP = 'IsApplicationStartup'
STARTUP_MSG = 'Silvanus Module Started Successfully.'

commands = []

command_definitions = [
    GenerateBoxCommand
]


def run(context):
    with appcontext() as app:
        for command in command_definitions:
            _units_type = app.activeProduct.fusionUnitsManager.distanceDisplayUnits
            _orientation = app.preferences.generalPreferences.defaultModelingOrientation
            cmd = command(app, units=_units_type, orientation=_orientation)

            button = CommandButton(app, cmd)
            commands.append(button)

        if context[APPLICATION_STARTUP] is False:
            app.userInterface.messageBox(STARTUP_MSG)
            logger.info(STARTUP_MSG)


def stop(context):
    with appcontext():
        for command in commands:
            command.stop()

        commands.clear()
    logger.info('Stopped Silvanus plugin.')
