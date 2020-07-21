import logging
from abc import ABC
from abc import abstractmethod

logger = logging.getLogger('silvanus').getChild(__name__)


class Fusion360Command(ABC):
    DESC = 'Run a Fusion 360 Command Button'
    ID = 'DefaultFusion360CommandButton'
    NAME = 'Fusion 360 Command Button'
    RESOURCES_FOLDER = 'resources'
    PANEL_LOCATION = 'SolidScriptsAddinsPanel'
    PREVIEW_FLAG = 'previewEnabledId'

    _dirty = False

    def __init__(self, app, *, units=None, orientation=None):
        self._app = app
        self._units_type = units
        self._orientation = orientation
        self._valid = True
        self._preview = False
        self._initialized = False
        self._first_run = True

    def on_change(self, cmd_input):
        pass

    def on_create(self, inputs):
        pass

    def on_destroy(self, inputs):
        pass

    def on_deactivate(self, inputs):
        pass

    @abstractmethod
    def on_execute(self, inputs):
        pass

    def on_preview(self, args):
        pass

    def on_validate(self, inputs):
        pass

    def post_validate_valid(self, inputs):
        pass

    def change(self, cmd_input):
        self._dirty = self.on_change(cmd_input)

    def create(self, inputs):
        self.on_create(inputs)
        self._initialized = True

    def destroy(self, inputs):
        self.on_destroy(inputs)

    def deactivate(self, inputs):
        self.on_deactivate(inputs)

    def execute(self, inputs):
        self.on_execute(inputs)

    def preview(self, args):
        if self._valid and self._dirty:
            self.post_validate_valid(args)
            self._dirty = False
            self.on_preview(args)
            self._first_run = False
            return True

        return False

    def validate(self, inputs):
        result = self.on_validate(inputs)
        self.valid = result
        return result
