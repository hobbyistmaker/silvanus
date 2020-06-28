import traceback

from .fusion.inputparser import parse_inputs
from .presenters import BasicBoxPresenter
from .fusion.inputconfiguration import inputs_from_units
from .usecases import DefineBoxUseCase
from .repositories import ApplicationRepository


def create(app, inputs):
    input_configuration = inputs_from_units(app)
    input_configuration.setup(inputs)


def execute(app, inputs):
    try:
        parse_inputs(app, inputs)
        application = ApplicationRepository(app, inputs)
        presenter = BasicBoxPresenter.create(application)
        definition = DefineBoxUseCase(presenter, application.parameters)
        definition.build()
    except:
        app.userInterface.messageBox(f'{traceback.format_exc()}')
