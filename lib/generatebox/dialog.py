import logging
from collections import defaultdict
from dataclasses import dataclass
from dataclasses import field
from typing import Any
from typing import Dict

from .config import config
from .ecs import Repository
from .entities import AxisFlag
from .entities import ConfigItem
from .entities import EnableInput
from .entities import EnableThicknessPair
from .entities import FingerOrientation
from .entities import FingerPatternType
from .entities import FingerType
from .entities import FingerWidthInput
from .entities import HeightInput
from .entities import InputProperty
from .entities import Inputs
from .entities import InsideInputs
from .entities import KerfInput
from .entities import DividerCountInput
from .entities import LengthInput
from .entities import MaxOffsetInput
from .entities import OutsidePanel
from .entities import OverrideInput
from .entities import PanelName
from .entities import PanelOrientation
from .entities import ParentPanel
from .entities import ReferencePointInput
from .entities import StartPanelOffset
from .entities import ThicknessInput
from .entities import WidthInput

import adsk.core

# noinspection SpellCheckingInspection
logger = logging.getLogger('silvanus.lib.generatebox.dialog')

yup = adsk.core.DefaultModelingOrientations.YUpModelingOrientation
zup = adsk.core.DefaultModelingOrientations.ZUpModelingOrientation


class Config:
    axes = config[ConfigItem.Axis]
    dimensions = config[ConfigItem.DimensionsGroup]
    inputs = config[ConfigItem.Inputs]
    panels = config[ConfigItem.Panels]
    parameters = config[ConfigItem.Parameters]
    overrides = config[ConfigItem.Overrides]
    enabled = config[ConfigItem.Enabled]
    simple_dividers = config[ConfigItem.SimpleDividerGroup]
    input_panels = { }

    axis_list = defaultdict(list)

    controls = { }

    thicknesses = []

    def clear(self):
        self.controls.clear()
        self.axis_list.clear()
        self.thicknesses.clear()


_config = Config()


class ParameterizedControl:

    def __init__(self, name, control):
        self._name = name
        self._control = control

    def __repr__(self):
        if self._control:
            return f'ParameterizedControl("{self._name}", {self.id}, {self.value}, {self.expression}, {self.unitType})\n'
        else:
            return f'\nParameterizedControl("{self._name}")\n'

    def __str__(self):
        return f'ParameterizedControl({self._name}, {self.id}, {self.value}, {self.expression}, {self.unitType})'

    @property
    def control(self):
        return self._control

    @property
    def enabled(self):
        return self._control.isEnabled

    @enabled.setter
    def enabled(self, value):
        self._control.isEnabled = value

    @property
    def expression(self):
        return self._control.expression

    @property
    def id(self):
        return self._control.id

    @property
    def name(self):
        return self._name

    @property
    def unitType(self):
        return getattr(self._control, 'unitType', '')

    @property
    def value(self):
        return self._control.value


@dataclass
class PanelInputs:
    enabled: bool
    axis: AxisFlag
    id: Any
    name: str
    orientation: int
    length: ParameterizedControl
    width: ParameterizedControl
    height: ParameterizedControl
    kerf: ParameterizedControl
    finger_width: ParameterizedControl
    profile_length: ParameterizedControl
    profile_width: ParameterizedControl
    offset: ParameterizedControl
    thickness: ParameterizedControl
    profile_transform: Any
    plane_selector: Any
    direction: Any
    face_selectors: Dict
    faces: Dict = field(default_factory=lambda: defaultdict(lambda: defaultdict(dict)))


class CreateDialog:
    config = _config
    repository = Repository()

    def __init__(self):
        self.panels = { }

        self._handlers = defaultdict(list)
        self._validators = { }
        self._message = { True: 'True', False: 'False' }
        self.valid = [True]
        self.preview = False
        self._ignore_updates = []
        self._run_preview = None

        self._error = None

    def clear(self):
        """ Clear out the dictionaries used for tracking input information. Since this class instance is
            persistent while the plugin is running, these dictionaries need to be cleared in between runs to
            prevent errors due to the deletion of inputs created in previous runs.
        """
        self.panels.clear()
        self._handlers.clear()
        self._validators.clear()
        self._message.clear()
        self.config.clear()
        self._error = None
        self._message = { True: '', False: '' }
        self.valid = [True]
        self._ignore_updates = []
        self.preview = False
        self._run_preview = None

    def create(self, inputs, is_metric, root, orientation):
        dimensions = self._create_dimension_group(inputs, is_metric)
        self._create_panel_table(dimensions.children, is_metric, root, orientation)
        self._create_divider_group(inputs, root, orientation)
        self._create_preview_button(inputs)

        self._add_minimum_axis_dimensions()
        self._add_maximum_kerf()
        self._add_minimum_finger_width()

        self._error = inputs.addTextBoxCommandInput('errorMessageCommandInput', '', '', 2, True)
        self._error.isVisible = False
        self._ignore_updates.append(self._error.id)

    def _create_preview_button(self, inputs):
        table = inputs.addTableCommandInput('buttonTableInput', 'Buttons', 0, '1:3')
        self._run_preview = table.commandInputs.addBoolValueInput('previewCommandButton', 'Preview', False, '', False)
        table.isEnabled = False
        table.minimumVisibleRows = 1
        table.maximumVisibleRows = 1
        table.addCommandInput(self._run_preview, 0, 0, 0, 0)
        self._add_validity_check(self._run_preview)

    def _create_dimension_group(self, inputs, is_metric):
        """ Create the default group of dimension inputs that define the overall dimensions of the box being
            created.
        """
        inputs_config = self.config.inputs
        dimension_group = inputs_config[Inputs.DimensionGroup]

        group = inputs.addGroupCommandInput(*dimension_group[InputProperty.Id])
        for item in self.config.dimensions:
            input_config = inputs_config[item]

            id_, label = input_config[InputProperty.Id]
            input_defaults = input_config[InputProperty.Defaults]
            name = self.config.parameters[item][ConfigItem.Name]

            spinner = ParameterizedControl(name, group.children.addFloatSpinnerCommandInput(
                    id_, label,
                    *input_defaults[is_metric]
            ))
            self._add_update_validator(spinner)

            self.config.controls[item] = spinner

        self._ignore_updates.append(group.id)
        return group

    def _create_divider_group(self, inputs, root, orientation):
        inputs_config = self.config.inputs
        simple_dividers_group = inputs_config[Inputs.SimpleDividerGroup]

        group = inputs.addGroupCommandInput(*simple_dividers_group[InputProperty.Id])
        for item in self.config.simple_dividers:
            input_config = inputs_config[item]

            id_, label = input_config[InputProperty.Id]
            name = self.config.parameters[item][ConfigItem.Name]

            spinner = ParameterizedControl(name, group.children.addIntegerSpinnerCommandInput(
                    id_, label,
                    0, 200, 1, 0
            ))
            self.config.controls[item] = spinner

            panel_entity = self.repository.create(
                    InsideInputs(),
                    ThicknessInput(Inputs.Thickness),
                    LengthInput(Inputs.Thickness),
                    WidthInput(Inputs.Width),
                    HeightInput(Inputs.Height),
                    KerfInput(Inputs.Kerf),
                    FingerWidthInput(Inputs.FingerWidth),
                    PanelOrientation(AxisFlag.Length),
                    MaxOffsetInput(Inputs.Length),
                    DividerCountInput(Inputs.LengthDivider)
            )

        self._ignore_updates.append(group.id)

    def _create_panel_table(self, inputs, is_metric, root, orientation):
        """ Creates the input table for the default outside panels that can be enabled by the user.
            Each panel can be enabled or disabled, and use the default material thickness, or an overridden thickness
            value.
        """
        default_thickness_control = self.config.controls[Inputs.Thickness]

        inputs_config = self.config.inputs
        table_input = inputs_config[Inputs.DimensionsTable]
        row_data = table_input[InputProperty.Rows]

        table = inputs.addTableCommandInput(*table_input[InputProperty.Id], *table_input[InputProperty.Defaults])
        table.maximumVisibleRows = len(row_data) + 1
        table.minimumVisibleRows = len(row_data) + 1
        table.isEnabled = False

        for title, col, span in table_input[InputProperty.Titles]:
            title_id, title_name = inputs_config[title][InputProperty.Id]
            title_label = f'<b>{title_name}</b>'
            title_input = table.commandInputs.addTextBoxCommandInput(title_id, title_name, title_label, 1, True)
            table.addCommandInput(title_input, 0, col, 0, span)

        for panel in row_data:
            row = inputs_config[panel.name]
            enable_input = inputs_config[panel.enable]
            override_input = inputs_config[panel.override]
            thickness_input = inputs_config[panel.thickness]
            name = self.config.parameters[panel.thickness][ConfigItem.Name]

            row_num = table.rowCount
            id_, label = row[InputProperty.Id]

            label_control = inputs.addTextBoxCommandInput(id_, label, f'<b>{label}</b>', 1, True)
            label_control.isEnabled = False

            c_enabled_id, c_enabled_label = enable_input[InputProperty.Id]
            enabled_control = inputs.addBoolValueInput(
                    c_enabled_id, c_enabled_label, True, '', enable_input[InputProperty.TurnedOn]
            )
            self.config.controls[panel.enable] = enabled_control

            c_override_id, c_override_label = override_input[InputProperty.Id]
            override_control = inputs.addBoolValueInput(c_override_id, c_override_label, True, '', False)
            override_control.isEnabled = override_input[InputProperty.Enabled]
            self.config.controls[panel.override] = override_control

            c_thickness_id, c_thickness_label = thickness_input[InputProperty.Id]
            thickness_control = inputs.addFloatSpinnerCommandInput(
                    c_thickness_id, c_thickness_label,
                    *thickness_input[InputProperty.Defaults][is_metric]
            )
            thickness_control.isEnabled = False
            p_control = ParameterizedControl(name, thickness_control)
            self.config.controls[panel.thickness] = default_thickness_control

            enable_thickness_pair = EnableThicknessPair(enabled_control, panel.thickness)
            enable_component = EnableInput(enabled_control)

            for finger_type, axes in panel.finger_types.items():
                for axis in axes:
                    self.repository.create(
                            enable_component,
                            OutsidePanel(),
                            PanelName(label),
                            OverrideInput(self.config.controls[panel.override]),
                            ThicknessInput(panel.thickness),
                            LengthInput(panel.length),
                            WidthInput(panel.width),
                            HeightInput(panel.height),
                            KerfInput(panel.kerf),
                            FingerWidthInput(panel.finger_width),
                            PanelOrientation(panel.orientation),
                            ReferencePointInput(*panel.reference_point),
                            MaxOffsetInput(panel.max_reference),
                            StartPanelOffset(0, '', ''),
                            FingerOrientation(axis),
                            FingerPatternType(finger_type),
                    )

            table.addCommandInput(label_control, row_num, 0, 0, 0)
            table.addCommandInput(enabled_control, row_num, 2, 0, 0)
            table.addCommandInput(override_control, row_num, 5, 0, 0)
            table.addCommandInput(thickness_control, row_num, 7, 0, 0)

            self.config.axis_list[panel.orientation].append((thickness_control, enabled_control))
            self.config.thicknesses.append(enable_thickness_pair)

            self._add_thickness_toggle(panel.thickness, override_control, default_thickness_control, p_control)
            self._add_enable_handler(enabled_control, override_control)
            self._add_enable_handler(override_control, thickness_control)
            self._add_panel_toggle(enable_component, enabled_control)
            self._add_follow_handler(default_thickness_control, thickness_control, enabled_control)
            self._add_follow_handler(default_thickness_control, thickness_control, override_control)
            self._add_disable_handler(enabled_control, thickness_control)
            self._add_update_thickness(thickness_control)

    def update(self, cmd_input):
        """ For each change handler registered for an input, execute the handler.
        """
        if cmd_input.id in self._ignore_updates:
            return False
        logger.debug(f'Updating: {cmd_input.id} is not in {self._ignore_updates}')

        self.valid = []

        handlers = self._handlers[cmd_input.id]
        for handler in handlers:
            handler()

        return True if cmd_input == self._run_preview else False

    def validate(self):
        """ Execute all registered validation handlers and return the combined "truthiness" result.
        """
        self.preview = False

        for validator in self._validators.values():
            validator()

        result = all(self.valid)

        if result:
            self._error.isVisible = False
            self.valid = [True]
            self.preview = True
            return True

        self.preview = False
        self.valid = [False]
        self._error.isVisible = True

        self._error.formattedText = self._message[False]

        return False

    def _add_validity_check(self, source):
        def check_validity():
            source.isEnabled = all(self.valid)

        self._validators[source.id] = check_validity

    def _add_update_validator(self, source):
        """ Add a validator to for the specified input that will force it to refresh on update.
        """

        def set_validator():
            def update_value():
                """ Read the value from the control to force a refresh in the dialog.
                    This fixes an issue where the user enters a number in the dialog without units;
                    the dialog will not show the units added until after it is validated. Once validated,
                    the units used for the previous value will be used again, unless the user specifies new units
                    in the input.
                """
                # noinspection PyUnusedLocal
                value = source.value
                return True

            self._validators[source.id] = update_value

        self._handlers[source.id].append(set_validator)

    def _add_enable_handler(self, source, dest):
        """ Add a change handler that will enable one input based on the truthiness of another input's value.
        """

        def enable_input():
            dest.isEnabled = bool(source.value) and source.isEnabled
            self.update(dest)

        self._handlers[source.id].append(enable_input)

    def _add_follow_handler(self, source, dest, guard):
        """ Add a change handler that will update the value of a destination input when a source input value is
            changed.
        """

        def update_value():
            dest.expression = dest.expression if dest.isEnabled and dest.value else source.expression
            self.update(dest)

        self._handlers[source.id].append(update_value)
        self._handlers[guard.id].append(update_value)

    def _add_disable_handler(self, source, dest):
        """ Add a change handler that zero's out a destination input value if a source input value is false.
        """

        def zero_thickness():
            dest.value = dest.value * source.value
            self.update(dest)

        zero_thickness()
        self._handlers[source.id].append(zero_thickness)

    def _add_minimum_axis_dimensions(self):
        """ Add a validation handler that checks if a dimension input is greater than the sum of a group of
            input's for panels along a specific axis.
        """

        def wrap(d, a):
            def check_minimum():
                msg = '<span style=" font-weight:600; color:#ff0000;">{}</span> should be greater than thickness of all {} panels.'
                total = sum([
                    source.value for source, enabled in filter(lambda t: bool(t[1].value), self.config.axis_list[a])
                ])
                result = d.value > total
                self._message[result] = msg.format(d.name.title(), a.name.lower())
                self.valid.append(result)

            return check_minimum

        for input_, axis in self.config.axes:
            dimension = self.config.controls[input_]
            self._handlers[dimension.id].append(wrap(dimension, axis))

    def _add_maximum_kerf(self):
        def check_kerf():
            thicknesses = [
                self.config.controls[panel.thickness].value for panel in
                filter(lambda p: p.enabled.value, self.config.thicknesses)
            ]
            if not len(thicknesses):
                self.valid.append(False)
                self._message[False] = 'A minimum of two connecting panels must be enabled.'
                return

            maximum = min(thicknesses)
            result = kerf.value < maximum
            self._message[
                result] = '<span style=" font-weight:600; color:#ff0000;">Kerf</span> should be smaller than the minimum panel thickness.'
            self.valid.append(result)

        kerf = self.config.controls[Inputs.Kerf]
        self._handlers[kerf.id].append(check_kerf)

        return check_kerf

    def _add_minimum_finger_width(self):
        def check_finger_width():
            thicknesses = [
                self.config.controls[panel.thickness].value for panel in
                filter(lambda p: p.enabled.value, self.config.thicknesses)
            ]
            if not len(thicknesses):
                self.valid.append(False)
                self._message[False] = 'A minimum of two connecting panels must be enabled.'
                return

            minimum = max(thicknesses)
            result = finger_width.value > minimum
            self._message[
                result] = '<span style=" font-weight:600; color:#ff0000;">Finger width</span> should be greater than material thickness.'
            self.valid.append(result)

        finger_width = self.config.controls[Inputs.FingerWidth]
        self._handlers[finger_width.id].append(check_finger_width)

        return check_finger_width

    def _add_thickness_toggle(self, id_, override, default, enabled):
        def toggle_thickness():
            selector = {
                True:  enabled,
                False: default
            }
            toggle = selector[override.value]
            self.config.controls[id_] = toggle
            self.update(toggle)

        self._handlers[override.id].append(toggle_thickness)

    def _add_panel_toggle(self, panel, enable):
        def toggle_panel():
            panel.enabled = enable.value

        self._handlers[enable.id].append(toggle_panel)

    def _add_update_thickness(self, thickness):
        self._handlers[thickness.id].append(self._add_maximum_kerf())
        self._handlers[thickness.id].append(self._add_minimum_finger_width())
