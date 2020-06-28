from abc import ABC, abstractmethod

import adsk.core
import adsk.fusion

from .config import inputs, input_defaults, config

BACK_PANEL_ENABLED_ID = inputs.back_panel_enabled.id
BACK_PANEL_THICKNESS_ID = inputs.back_panel_thickness.id
FRONT_PANEL_ENABLED_ID = inputs.front_panel_enabled.id
FRONT_PANEL_THICKNESS_ID = inputs.front_panel_thickness.id
RIGHT_PANEL_ENABLED_ID = inputs.right_panel_enabled.id
RIGHT_PANEL_THICKNESS_ID = inputs.right_panel_thickness.id
LEFT_PANEL_ENABLED_ID = inputs.left_panel_enabled.id
LEFT_PANEL_THICKNESS_ID = inputs.left_panel_thickness.id
BOTTOM_PANEL_ENABLED_ID = inputs.bottom_panel_enabled.id
BOTTOM_PANEL_THICKNESS_ID = inputs.bottom_panel_thickness.id
TOP_PANEL_ENABLED_ID = inputs.top_panel_enabled.id
TOP_PANEL_THICKNESS_ID = inputs.top_panel_thickness.id
GROUP_COMMAND_ID = inputs.dimensions_group.id
GROUP_COMMAND_TITLE = inputs.dimensions_group.name
LENGTH_COMMAND_ID = inputs.length_command.id
LENGTH_INPUT_NAME = inputs.length_command.name
WIDTH_COMMAND_ID = inputs.width_command.id
WIDTH_INPUT_NAME = inputs.width_command.name
HEIGHT_COMMAND_ID = inputs.height_command.id
HEIGHT_INPUT_NAME = inputs.height_command.name
DEFAULT_THICKNESS_COMMAND_ID = inputs.thickness_command.id
DEFAULT_THICKNESS_NAME = inputs.thickness_command.name
DEFAULT_FINGER_WIDTH_ID = inputs.finger_width_command.id
DEFAULT_FINGER_WIDTH_NAME = inputs.finger_width_command.name
KERF_COMMAND_ID = inputs.kerf_command.id
KERF_INPUT_NAME = inputs.kerf_command.name

def parsable_inputs():
    inputs = config['inputs']
    return [inputs[input_] for input_ in inputs if 'parameter' in inputs[input_] and inputs[input_]['parameter']]


def inputs_from_units(app):
    product = app.activeProduct
    design = adsk.fusion.Design.cast(product)
    use_metric = design.fusionUnitsManager.distanceDisplayUnits < 3

    return MetricInputs(app) if use_metric else ImperialInputs(app)


class InputConfiguration(ABC):

    def __init__(self, app):
        product = app.activeProduct
        design = adsk.fusion.Design.cast(product)
        units = design.fusionUnitsManager

        self.units = units.defaultLengthUnits
        self.convert = units.convert

    @property
    @abstractmethod
    def length_defaults(self):
        ...

    @property
    @abstractmethod
    def width_defaults(self):
        ...

    @property
    @abstractmethod
    def height_defaults(self):
        ...

    @property
    @abstractmethod
    def thickness_defaults(self):
        ...

    @property
    @abstractmethod
    def finger_defaults(self):
        ...

    @property
    @abstractmethod
    def kerf_defaults(self):
        ...

    @abstractmethod
    def float_defaults(self, value): pass

    def setup(self, inputs_):
        self._setup_dimension_inputs(inputs_)
        self._setup_panel_dimensions(inputs_)

        inputs_.addBoolValueInput(
                f'previewEnabledId', 'Preview', True, '', False
        )

    def _setup_panel_dimensions(self, inputs_):
        group_input = inputs_.addGroupCommandInput('panelDimensionsGroupId', 'Panel Thickness')
        group_input.isExpanded = False
        children = group_input.children
        table_input = children.addTableCommandInput('dimensionsTableId', 'Table', 3, '1:1:2')
        table_input.maximumVisibleRows = 7
        table_input.minimumVisibleRows = 7

        cmd_inputs = adsk.core.CommandInputs.cast(table_input.commandInputs)

        self._add_panel_thickness_title_labels(cmd_inputs, table_input)
        self._add_table_thickness(cmd_inputs, table_input, 'Top', 1, False)
        self._add_table_thickness(cmd_inputs, table_input, 'Bottom', 2)
        self._add_table_thickness(cmd_inputs, table_input, 'Left', 3)
        self._add_table_thickness(cmd_inputs, table_input, 'Right', 4)
        self._add_table_thickness(cmd_inputs, table_input, 'Front', 5)
        self._add_table_thickness(cmd_inputs, table_input, 'Back', 6)

    def _add_panel_thickness_title_labels(self, cmd_inputs, table_input):
        panel_title = self._create_column_title(cmd_inputs, 'Panel')
        enabled_title = self._create_column_title(cmd_inputs, 'Enabled')
        thickness_title = self._create_column_title(cmd_inputs, 'Thickness')
        titles = [panel_title, enabled_title, thickness_title]
        for i, title in enumerate(titles):
            table_input.addCommandInput(title, 0, i)

    @staticmethod
    def _create_column_title(cmd_inputs, name):
        panel_title = cmd_inputs.addStringValueInput(name, name, name)
        panel_title.isReadOnly = True
        return panel_title

    def _add_table_thickness(self, cmd_inputs, table_input, name, row, enabled=True):
        top_string_input = cmd_inputs.addStringValueInput(name, name, name)
        top_string_input.isReadOnly = True
        enable_input = cmd_inputs.addBoolValueInput(
                f'{name.lower()}PanelEnabledId', name, True, '', enabled
        )
        dimension_input = cmd_inputs.addFloatSpinnerCommandInput(
                f'{name.lower()}PanelThicknessId',
                '',
                *self.thickness_defaults
        )
        table_input.addCommandInput(top_string_input, row, 0)
        table_input.addCommandInput(enable_input, row, 1)
        table_input.addCommandInput(dimension_input, row, 2)

    def _setup_dimension_inputs(self, inputs_):
        group = inputs_.addGroupCommandInput(GROUP_COMMAND_ID, GROUP_COMMAND_TITLE)
        group.isExpanded = True
        group_inputs = group.children
        group_inputs.addFloatSpinnerCommandInput(
                LENGTH_COMMAND_ID,
                LENGTH_INPUT_NAME,
                *self.length_defaults
        )
        group_inputs.addFloatSpinnerCommandInput(
                WIDTH_COMMAND_ID,
                WIDTH_INPUT_NAME,
                *self.width_defaults
        )
        group_inputs.addFloatSpinnerCommandInput(
                HEIGHT_COMMAND_ID,
                HEIGHT_INPUT_NAME,
                *self.height_defaults
        )
        group_inputs.addFloatSpinnerCommandInput(
                DEFAULT_THICKNESS_COMMAND_ID,
                DEFAULT_THICKNESS_NAME,
                *self.thickness_defaults
        )
        group_inputs.addFloatSpinnerCommandInput(
                DEFAULT_FINGER_WIDTH_ID,
                DEFAULT_FINGER_WIDTH_NAME,
                *self.finger_defaults
        )

        group_inputs.addFloatSpinnerCommandInput(
                KERF_COMMAND_ID,
                KERF_INPUT_NAME,
                *self.kerf_defaults
        )


class ImperialInputs(InputConfiguration):

    @property
    def length_defaults(self):
        return self.float_defaults(input_defaults.imperial.length)

    @property
    def width_defaults(self):
        return self.float_defaults(input_defaults.imperial.width)

    @property
    def height_defaults(self):
        return self.float_defaults(input_defaults.imperial.height)

    @property
    def thickness_defaults(self):
        return self.float_defaults(input_defaults.imperial.thickness)

    @property
    def finger_defaults(self):
        return self.float_defaults(input_defaults.imperial.finger_width)

    def float_defaults(self, value):
        return (self.units,
                self.convert(.0625, 'in', self.units),
                self.convert(24, 'in', self.units),
                self.convert(0.1, 'in', self.units),
                self.convert(value, 'in', self.units)
                )

    @property
    def kerf_defaults(self):
        return (self.units,
                self.convert(0, 'in', self.units),
                self.convert(1, 'in', self.units),
                self.convert(0.005, 'in', self.units),
                self.convert(0, 'in', self.units)
                )


class MetricInputs(InputConfiguration):

    @property
    def length_defaults(self):
        return self.float_defaults(input_defaults.metric.length)

    @property
    def width_defaults(self):
        return self.float_defaults(input_defaults.metric.width)

    @property
    def height_defaults(self):
        return self.float_defaults(input_defaults.metric.height)

    @property
    def thickness_defaults(self):
        return self.float_defaults(input_defaults.metric.thickness)

    @property
    def finger_defaults(self):
        return self.float_defaults(input_defaults.metric.finger_width)

    def float_defaults(self, value):
        return (self.units,
                self.convert(0.5, 'mm', self.units),
                self.convert(2500, 'mm', self.units),
                self.convert(0.1, 'mm', self.units),
                self.convert(value, 'mm', self.units)
                )

    @property
    def kerf_defaults(self):
        return (self.units,
                self.convert(0, 'mm', self.units),
                self.convert(12.7, 'mm', self.units),
                self.convert(0.127, 'mm', self.units),
                self.convert(0, 'mm', self.units)
                )
