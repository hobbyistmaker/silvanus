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

    # these should be set by the run() method
    _units_type = None
    _orientation = None
    _app = None

    def on_destroy(self, inputs):
        pass

    def on_deactivate(self, inputs):
        pass

    @abstractmethod
    def on_execute(self, inputs):
        pass

    @abstractmethod
    def on_change(self, cmd_input):
        pass

    @abstractmethod
    def on_validate(self, inputs):
        pass

    def on_create(self, inputs):
        self._units_type = self._app.activeProduct.fusionUnitsManager.distanceDisplayUnits
        self._orientation = self._app.preferences.generalPreferences.defaultModelingOrientation
        self._on_create(inputs)

    @abstractmethod
    def _on_create(self, inputs):
        pass

    @abstractmethod
    def on_preview(self, inputs):
        pass
