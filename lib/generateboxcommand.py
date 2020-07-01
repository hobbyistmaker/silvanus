import logging

from .common import Fusion360Command
from .generatebox.dialog import CreateDialog
from .generatebox.core import Core

# noinspection SpellCheckingInspection

logger = logging.getLogger('silvanus.lib.generateboxcommand')


class GenerateBoxCommand(Fusion360Command):
    DESC = 'Create a parametric box with finger joints.'
    ID = 'SilvanusGenerateBoxCommandId'
    DIALOG = 'Create Box'

    _dialog = CreateDialog()
    _core = Core(_dialog)

    def on_destroy(self, args):
        """ Clear the entities and the input dialog after each run of the command.
        """
        self._dialog.clear()
        self._core.clear()

    def _on_create(self, command):
        logger.debug('Creating "Create Box" dialog.')
        root = self._app.activeProduct.rootComponent
        preferences = self._app.preferences.generalPreferences
        orientation = preferences.defaultModelingOrientation

        self._configure_dialog(command)
        self._dialog.create(command.commandInputs, self._units_type < 3, root, orientation)

        logger.debug('Dialog creation completed.')

    @staticmethod
    def _configure_dialog(command):
        """ Define the minimum dimensions for the input dialog. This tries to limit the appearance of
            scrollbars in the dialog.
        """
        command.setDialogMinimumSize(329, 425)
        command.isRepeatable = False
        command.okButtonText = 'Create'

    def on_change(self, cmd_input):
        self._dialog.update(cmd_input)

    def on_execute(self, inputs):
        root = self._app.activeProduct.rootComponent
        preferences = self._app.preferences.generalPreferences
        orientation = preferences.defaultModelingOrientation

        self._core.execute(panels=self._dialog.panels, orientation=orientation, component=root)

    def on_preview(self, inputs):
        root = self._app.activeProduct.rootComponent
        preferences = self._app.preferences.generalPreferences
        orientation = preferences.defaultModelingOrientation

        self._core.preview(panels=self._dialog.panels, orientation=orientation, component=root)

    def on_validate(self, inputs):
        result = self._dialog.validate()
        return result
