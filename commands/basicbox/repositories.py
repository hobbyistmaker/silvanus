from collections import namedtuple

import adsk.core
import adsk.fusion

from .usecases import DefineParametersUseCase
from .fusion import config

AxisFunctions = namedtuple('AxisFunctions', 'min max')
FParameterTuple = namedtuple('FParameterTuple', 'value expression')


class ApplicationRepository:

    def __init__(self, app, inputs):
        self.app = app
        self.is_saved = app.activeDocument.isSaved
        self.message = app.userInterface.messageBox

        design = adsk.fusion.Design.cast(app.activeProduct)
        self.root_component = design.rootComponent
        self.active_component = design.activeComponent
        self.timeline = design.timeline
        self.start_position = self.timeline.markerPosition

        self.units_manager = design.fusionUnitsManager
        self.default_units = self.units_manager.defaultLengthUnits

        self.all_parameters = design.allParameters
        self.user_parameters = design.userParameters

        preferences = app.preferences.generalPreferences
        orientation = preferences.defaultModelingOrientation
        self.y_up = orientation == adsk.core.DefaultModelingOrientations.YUpModelingOrientation
        self.parameters = self.parse_parameters(inputs)

    def parse_parameters(self, inputs):
        fusion_parameters = FusionParameterRepository(self.all_parameters, inputs)
        box_parameters = DefineParametersUseCase(fusion_parameters)
        return box_parameters.build()


class Faces:

    def __init__(self, *, body, repository):
        assert body is not None
        assert repository is not None

        self.body = body
        self.repository = repository

    @property
    def faces(self):
        assert self.body.isValid

        return self.body.faces

    @property
    def left(self):
        return self.repository.left(self.faces)

    @property
    def right(self):
        return self.repository.right(self.faces)

    @property
    def front(self):
        return self.repository.front(self.faces)

    @property
    def back(self):
        return self.repository.back(self.faces)

    @property
    def top(self):
        return self.repository.top(self.faces)

    @property
    def bottom(self):
        return self.repository.bottom(self.faces)


class FacesRepository:

    def __init__(self, app):
        assert app is not None
        assert app.y_up == 0 or app.y_up == 1

        self.y_up = app.y_up

        z_axis = AxisFunctions(
                lambda faces: sorted(faces, key=lambda f: f.centroid.z)[0],
                lambda faces: sorted(faces, key=lambda f: f.centroid.z, reverse=True)[0]
        )

        y_axis = AxisFunctions(
                lambda faces: sorted(faces, key=lambda f: f.centroid.y)[0],
                lambda faces: sorted(faces, key=lambda f: f.centroid.y, reverse=True)[0]
        )

        x_axis = AxisFunctions(
                lambda faces: sorted(faces, key=lambda f: f.centroid.x)[0],
                lambda faces: sorted(faces, key=lambda f: f.centroid.x, reverse=True)[0]
        )

        self.left, self.right = (x_axis.min, x_axis.max)

        self.bottom, self.top = (
                (y_axis.min, y_axis.max)
                if self.y_up else
                (z_axis.min, z_axis.max)
        )

        self.front, self.back = (
                (z_axis.min, z_axis.max)
                if self.y_up else
                (y_axis.min, y_axis.max)
        )

    def faces(self, body):
        assert body is not None

        return Faces(body=body, repository=self)


class FusionParameterRepository:

    def __init__(self, parameters, inputs):
        self.parameters = parameters
        self.inputs = inputs

        self.length = self._get_parameter('length')
        self.width = self._get_parameter('width')
        self.height = self._get_parameter('height')
        self.thickness = self._get_parameter('thickness')
        self.finger_width = self._get_parameter('dfingerw')
        self.kerf = self._get_parameter('kerf', allow_none=True)

        assert self.length is not None
        assert self.width is not None
        assert self.height is not None
        assert self.thickness is not None
        assert self.finger_width is not None

        self.options = {
                'top': {
                        'enable': self._get_boolean_input(config.inputs.top_panel_enabled.id),
                        'thickness': self._get_parameter(config.inputs.top_panel_thickness.parameter)
                },
                'bottom': {
                        'enable': self._get_boolean_input(config.inputs.bottom_panel_enabled.id),
                        'thickness': self._get_parameter(config.inputs.bottom_panel_thickness.parameter)
                },
                'left': {
                        'enable': self._get_boolean_input(config.inputs.left_panel_enabled.id),
                        'thickness': self._get_parameter(config.inputs.left_panel_thickness.parameter)
                },
                'right': {
                        'enable': self._get_boolean_input(config.inputs.right_panel_enabled.id),
                        'thickness': self._get_parameter(config.inputs.right_panel_thickness.parameter)
                },
                'front': {
                        'enable': self._get_boolean_input(config.inputs.front_panel_enabled.id),
                        'thickness': self._get_parameter(config.inputs.front_panel_thickness.parameter)
                },
                'back': {
                        'enable': self._get_boolean_input(config.inputs.back_panel_enabled.id),
                        'thickness': self._get_parameter(config.inputs.back_panel_thickness.parameter)
                }
        }

    def _get_parameter(self, name, *, allow_none=False):
        param = self.parameters.itemByName(name)

        if not allow_none:
            assert param is not None
            assert param.value is not None
            assert param.name is not None

        return FParameterTuple(param.value, param.name) if param else FParameterTuple(None, None)

    def _get_input(self, name):
        param = self.inputs.itemById(name)

        assert param is not None
        assert param.value is not None

        return FParameterTuple(param.value, str(param.value))

    def _get_boolean_input(self, name):
        result = self.inputs.itemById(name).value

        assert result is not None

        return FParameterTuple(result, result)
