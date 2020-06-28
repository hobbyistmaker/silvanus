import logging
from collections import defaultdict

from .common import Fusion360Command
from .generatebox.config import config
from .generatebox.entities import ConfigItem
from .generatebox.entities import InputProperty
from .generatebox.entities import Inputs
from .generatebox.core import Core

# noinspection SpellCheckingInspection
logger = logging.getLogger('silvanus.lib.generateboxcommand')


class Config:
    axes = config[ConfigItem.Axis]
    dimensions = config[ConfigItem.DimensionsGroup]
    inputs = config[ConfigItem.Inputs]
    panels = config[ConfigItem.Panels]
    planes = config[ConfigItem.Planes]
    parameters = config[ConfigItem.Parameters]

    axis_list = defaultdict(list)

    controls = { }

    thicknesses = []

    def clear(self):
        self.controls.clear()
        self.axis_list.clear()
        self.thicknesses.clear()


_config = Config()


class CreateDialog:
    _config = _config

    def __init__(self):
        logger.debug(f'Starting dialog with {self._config.panels}')
        self._handlers = defaultdict(list)
        self._validators = { }
        self._message = { True: 'True', False: 'False' }
        self.valid = [True]

        self._error = None

    def clear(self):
        """ Clear out the dictionaries used for tracking input information. Since this class instance is
            persistent while the plugin is running, these dictionaries need to be cleared in between runs to
            prevent errors due to the deletion of inputs created in previous runs.
        """
        self._handlers.clear()
        self._validators.clear()
        self._message.clear()
        self._config.clear()
        self._error = None
        self._message = { True: '', False: '' }
        self.valid = [True]

    def create(self, inputs, is_metric):
        self._create_dimension_group(inputs, is_metric)
        self._create_panel_table(inputs, is_metric)

        self._add_minimum_axis_dimensions()
        self._add_maximum_kerf()
        self._add_minimum_finger_width()

        self._error = inputs.addTextBoxCommandInput('errorMessageCommandInput', '', '', 2, True)
        self._error.isVisible = False

    def _create_dimension_group(self, inputs, is_metric):
        """ Create the default group of dimension inputs that define the overall dimensions of the box being
            created.
        """
        inputs_config = self._config.inputs
        dimension_group = inputs_config[Inputs.DimensionGroup]

        group = inputs.addGroupCommandInput(*dimension_group[InputProperty.Id])
        for item in self._config.dimensions:
            input_config = inputs_config[item]

            id_, label = input_config[InputProperty.Id]
            input_defaults = input_config[InputProperty.Defaults]

            spinner = group.children.addFloatSpinnerCommandInput(
                    id_, label,
                    *input_defaults[is_metric]
            )
            self._add_update_validator(spinner)

            self._config.controls[item] = spinner

        thickness = self._config.controls[Inputs.Thickness]
        self._config.thicknesses.append((thickness, thickness))

    def _create_panel_table(self, inputs, is_metric):
        """ Creates the input table for the default outside panels that can be enabled by the user.
            Each panel can be enabled or disabled, and use the default material thickness, or an overridden thickness
            value.
        """
        thickness_control = self._config.controls[Inputs.Thickness]

        inputs_config = self._config.inputs
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

        for row_id, enable, override, thickness in row_data:
            row = inputs_config[row_id]
            enable_input = inputs_config[enable]
            override_input = inputs_config[override]
            thickness_input = inputs_config[thickness]

            row_num = table.rowCount
            id_, label = row[InputProperty.Id]

            c_label = inputs.addTextBoxCommandInput(id_, label, f'<b>{label}</b>', 1, True)
            c_label.isEnabled = False

            c_enabled_id, c_enabled_label = enable_input[InputProperty.Id]
            c_enabled = inputs.addBoolValueInput(
                    c_enabled_id, c_enabled_label, True, '', enable_input[InputProperty.TurnedOn]
            )
            self._config.controls[enable] = c_enabled

            c_override_id, c_override_label = override_input[InputProperty.Id]
            c_override = inputs.addBoolValueInput(c_override_id, c_override_label, True, '', False)
            c_override.isEnabled = override_input[InputProperty.Enabled]
            self._config.controls[override] = c_override

            c_thickness_id, c_thickness_label = thickness_input[InputProperty.Id]
            c_thickness = inputs.addFloatSpinnerCommandInput(
                    c_thickness_id, c_thickness_label,
                    *thickness_input[InputProperty.Defaults][is_metric]
            )
            c_thickness.isEnabled = False
            self._config.controls[thickness] = c_thickness

            table.addCommandInput(c_label, row_num, 0, 0, 0)
            table.addCommandInput(c_enabled, row_num, 2, 0, 0)
            table.addCommandInput(c_override, row_num, 5, 0, 0)
            table.addCommandInput(c_thickness, row_num, 7, 0, 0)

            axis = thickness_input[InputProperty.Axis]
            self._config.axis_list[axis].append((c_thickness, c_enabled))

            self._config.thicknesses.append((c_thickness, c_enabled))

            self._add_enable_handler(c_enabled, c_override)
            self._add_enable_handler(c_override, c_thickness)
            self._add_follow_handler(thickness_control, c_thickness, c_enabled)
            self._add_follow_handler(thickness_control, c_thickness, c_override)
            self._add_disable_handler(c_enabled, c_thickness)
            self._add_update_validator(c_thickness)

    def update(self, cmd_input):
        """ For each change handler registered for an input, execute the handler.
        """
        self.valid = [True]

        handlers = self._handlers[cmd_input.id]
        for handler in handlers:
            handler()

    def validate(self):
        """ Execute all registered validation handlers and return the combined "truthiness" result.
        """
        for validator in self._validators.values():
            validator()

        result = all(self.valid)

        self._error.formattedText = self._message[False]
        self._error.isVisible = not result
        self.valid = [result]

        return result

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
        """ Add a change handler that will enable one input based on the boolean truthiness of another input's value.
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
                msg = '<b style="color:red">{}</b> should be greater than thickness of all {} panels.'
                total = sum([
                    source.value for source, enabled in filter(lambda t: bool(t[1].value), self._config.axis_list[a])
                ])
                result = d.value > total
                self._message[result] = msg.format(d.name, a.name.lower())
                self.valid.append(result)

            return check_minimum

        for input_, axis in self._config.axes:
            dimension = self._config.controls[input_]
            self._handlers[dimension.id].append(wrap(dimension, axis))

    def _add_maximum_kerf(self):
        def check_kerf():
            maximum = min([
                thickness.value for thickness, enabled in filter(lambda t: bool(t[1].value), self._config.thicknesses)
            ])
            result = kerf.value < maximum
            self._message[result] = '<b style="color:red">Kerf</b> should be smaller than the minimum panel thickness.'
            self.valid.append(result)

        kerf = self._config.controls[Inputs.Kerf]
        self._handlers[kerf.id].append(check_kerf)

    def _add_minimum_finger_width(self):
        def check_finger_width():
            minimum = min([
                thickness.value for thickness, enabled in filter(lambda t: bool(t[1].value), self._config.thicknesses)
            ])
            result = finger_width.value > minimum
            self._message[result] = '<b style="color:red">Finger width</b> should be greater than material thickness.'
            self.valid.append(result)

        finger_width = self._config.controls[Inputs.FingerWidth]
        self._handlers[finger_width.id].append(check_finger_width)


class GenerateBoxCommand(Fusion360Command):
    DESC = 'Create a parametric box with finger joints.'
    ID = 'SilvanusGenerateBoxCommandId'
    DIALOG = 'Create Box'

    _config = _config
    _dialog = CreateDialog()
    _core = Core(_config)

    def on_destroy(self, args):
        """ Clear the entities and the input dialog after each run of the command.
        """
        logger.debug('Clearing dialog.')
        self._dialog.clear()
        self._core.clear()

    def _on_create(self, command):
        logger.debug('Creating "Create Box" dialog.')
        self._configure_dialog(command)
        self._dialog.create(command.commandInputs, self._units_type < 3)

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
        logger.debug('Command on_change event fired.')
        logger.debug(f'On change command id: {cmd_input.id}')
        self._dialog.update(cmd_input)
        logger.debug('Command on_change event finished.')

    def on_execute(self, inputs):
        root = self._app.activeProduct.rootComponent
        preferences = self._app.preferences.generalPreferences
        orientation = preferences.defaultModelingOrientation

        self._core.execute(orientation, root)

    def on_preview(self, inputs):
        root = self._app.activeProduct.rootComponent
        preferences = self._app.preferences.generalPreferences
        orientation = preferences.defaultModelingOrientation

        self._core.preview(orientation, root)

    def on_validate(self, inputs):
        logger.debug('Command on_validate event fired.')
        result = self._dialog.validate()
        logger.debug(f'Command on_validate event finished {result}.')
        return result
