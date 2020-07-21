import logging

from .common import Fusion360Command
from .generatebox.dialog import CreateDialog
from .generatebox.process import Core

# noinspection SpellCheckingInspection

logger = logging.getLogger('silvanus.lib.generateboxcommand')


class GenerateBoxCommand(Fusion360Command):
    DESC = 'Create a parametric box with finger joints.'
    ID = 'SilvanusGenerateBoxCommandId'
    DIALOG = 'Create Box'

    _dialog = CreateDialog()
    _core = Core(_dialog)

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.root = None
        self.preferences = None
        self.orientation = None

    def on_destroy(self, args):
        """ Clear the entities and the input dialog after each run of the command.
        """
        self._dialog.clear()
        self._core.clear()

    def on_create(self, command):
        logger.debug('Creating "Create Box" dialog.')
        self.root = self._app.activeProduct.rootComponent
        self.preferences = self._app.preferences.generalPreferences
        self.orientation = self.preferences.defaultModelingOrientation

        self._configure_dialog(command)
        self._dialog.create(command.commandInputs, self._units_type < 3, self.root, self.orientation)
        logger.debug('Dialog creation completed.')

    @staticmethod
    def _configure_dialog(command):
        """ Define the minimum dimensions for the input dialog. This tries to limit the appearance of
            scrollbars in the dialog.
        """
        command.setDialogMinimumSize(329, 425)
        command.isRepeatable = False
        command.okButtonText = 'Create'

    def on_change(self, args):
        cmd_input = args.input
        logger.debug(f'On Change Notification for {cmd_input.id}')

        return self._dialog.update(cmd_input)

    def on_execute(self, inputs):
        self._core.update(orientation=self.orientation)
        self._core.execute(orientation=self.orientation, component=self.root)

    def on_preview(self, args):
        if not self._dialog.preview:
            return

        self._core.preview(orientation=self.orientation, component=self.root)

    def on_validate(self, inputs):
        return self._dialog.validate()

    def post_validate_valid(self, inputs):
        self._core.update(orientation=self.orientation)
