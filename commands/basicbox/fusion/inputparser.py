import traceback

import adsk.core
import adsk.fusion
from adsk.core import ValueInput as Vi

from .inputconfiguration import parsable_inputs

class InputParser:

    def __init__(self, app, inputs_):
        self.app = app
        self.inputs = inputs_
        self.units = self.app.activeProduct.fusionUnitsManager.defaultLengthUnits

    def parse(self, parameter, description=''):
        input_ = self.inputs.itemById(parameter['id'])
        if input_:
            return self.__find_or_create_parameter(parameter, input_, description=description)
        return None

    @staticmethod
    def __expression_value(input_):
        if isinstance(input_, Vi):
            return input_
        elif isinstance(input_, adsk.fusion.Parameter):
            return Vi.createByString(f'{input_.expression}')
        elif isinstance(input_, str):
            return Vi.createByString(input_)
        elif isinstance(input_, (int, float)):
            return Vi.createByReal(input_)
        elif isinstance(input_, adsk.core.CommandInput):
            return Vi.createByString(input_.expression)
        else:
            return NotImplemented(f'Cannot process input type: {input_}')

    def __find_or_create_parameter(self, parameter, input_, description=''):
        try:
            existing_parameter = self.app.activeProduct.allParameters.itemByName(parameter['parameter'])
            return (
                    self.__update_parameter(existing_parameter,
                                            input_,
                                            description=description,
                                            default=parameter.get('default', None))
                    if existing_parameter else
                    self.__create_parameter(parameter['parameter'],
                                            input_,
                                            description=description,
                                            default=parameter.get('default', None))
            )
        except Exception as e:
            trace = traceback.format_exc()
            self.app.userInterface.messageBox(f'{trace}')

    def __create_parameter(self, name, input_, description='', default=None):
        other = self.app.activeProduct.allParameters.itemByName(default) if default else None
        current_expression = self.__expression_value(input_)

        other_expression_matches = other and other.expression == current_expression.stringValue
        expression = self.__expression_value(other.name) if other_expression_matches else current_expression

        return self.app.activeProduct.userParameters.add(name, expression,
                                                         self.__default_units(input_), description)

    def __update_parameter(self, existing, input_, description='', default=None):
        other = self.app.activeProduct.allParameters.itemByName(default) if default else None
        current_expression = self.__expression_value(input_).stringValue
        expression = other.name if other and other.expression == current_expression else current_expression

        existing.expression = expression
        existing.unit = self.__default_units(input_)
        existing.description = description

        return existing

    def __default_units(self, input_):
        return getattr(input_, 'unitType', self.units)


def parse_inputs(app, inputs):
    parser = InputParser(app, inputs)

    parsable = parsable_inputs()
    for key in parsable:
        parser.parse(key)
