import logging

import adsk.core

from .ecs import Process
from .entities import ConfigItem
from .entities import Parameter

# noinspection SpellCheckingInspection
logger = logging.getLogger('silvanus.lib.generatebox.save')


class SaveParameters(Process):

    def __init__(self):
        self._app = adsk.core.Application.get()
        self._parameters = self._app.activeProduct.allParameters
        self._store = self._app.activeProduct.userParameters

    def process(self):
        parameters = self._convert_dimension_parameters()

        for parameter in filter(lambda p: p.value > 0, parameters):
            self._find_or_create_parameter(parameter.name, parameter)

    def _convert_dimension_parameters(self):
        """ Convert the dimension controls to parameters, and override individual panel thickness parameters
            if required.
        """
        parameters = set()
        # Convert dimensions with parameters to straight parameters
        for key, parameter in filter(lambda p: p[1][ConfigItem.Enabled], self._repository.parameters.items()):
            control = parameter[ConfigItem.Control]
            name = parameter[ConfigItem.Name]
            parameters.add(Parameter(name, control.value, control.unitType))
        return parameters

    def _find_or_create_parameter(self, name, parameter):
        """ Find the parameter with the given name or create that parameter and return it.
        """
        parameter_selector = {
            True:  lambda e, p: self._update_parameter(e, p),
            False: lambda e, p: self._create_parameter(p)
        }

        existing_parameter = self._parameters.itemByName(name)
        return parameter_selector[bool(existing_parameter)](existing_parameter, parameter)

    def _create_parameter(self, parameter):
        """ Create the given user parameter in Fusion360.
        """
        value = adsk.core.ValueInput.createByReal(parameter.value)
        return self._store.add(parameter.name, value, parameter.unitType, '')

    def _update_parameter(self, existing, parameter):
        """ Update the existing parameter with the current values specified by the user.
        """
        existing.value = parameter.value
        existing.unit = parameter.unitType
