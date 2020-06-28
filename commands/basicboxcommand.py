from .common.fusion360command import Fusion360Command
from .common.createdeventhandler import CreatedEventHandler

from .basicbox.start import create, execute


class BasicBoxCommand(Fusion360Command):

    COMMAND_DESC = 'Create a parametric box with finger joints.'
    COMMAND_ID = 'SilvanusBasicBoxCommandId'
    COMMAND_NAME = 'New Basic Box'
    RESOURCES_FOLDER = 'resources'
    SCRIPTS_ID = 'SolidScriptsAddinsPanel'
    PREVIEW_ID = 'previewEnabledId'
    COMMAND_HANDLER = CreatedEventHandler

    @property
    def preview_id(self):
        return self.PREVIEW_ID

    @property
    def command_id(self):
        return self.COMMAND_ID

    @property
    def scripts_id(self):
        return self.SCRIPTS_ID

    @property
    def command_desc(self):
        return self.COMMAND_DESC

    @property
    def resources(self):
        # return self.RESOURCES_FOLDER
        return None

    @property
    def command_name(self):
        return self.COMMAND_NAME

    def _on_create(self, inputs):
        create(self.app, inputs)

    def _on_change(self, inputs):
        pass

    def _on_validate(self, inputs):
        pass

    def _on_preview(self, inputs):
        self._execute(inputs)

    def _on_execute(self, inputs):
        self._execute(inputs)

    def _execute(self, inputs):
        execute(self.app, inputs)
