import logging

from .eventhandlers import CreatedEventHandler
from .uicontext import uicontext

# noinspection SpellCheckingInspection
level = 'lib.common.commandbutton'
logger = logging.getLogger('silvanus').getChild(level)


class CommandButton:

    def __init__(self, app, command):
        self.app = app
        self.command = command
        self.handlers = []
        self.created_handler = CreatedEventHandler

        self.start()

    def start(self):
        self.del_command_button()
        self.add_command_button()

    def add_command_button(self):
        self.add_control_to_panel()
        self.register_handler()

    def add_control_to_panel(self):
        if not self.command_control:
            self.scripts_panel.controls.addCommand(self.find_command_definition())

    def register_handler(self):
        handler = self.created_handler(self, self.command.PREVIEW_FLAG)
        self.handlers.append(handler)

        cmd_def = self.find_command_definition()
        if not cmd_def.commandCreated.add(handler):
            logging.error('Unable to register handler for command')
            raise RuntimeError('Unable to register handler for command')

    def stop(self):
        self.del_command_button()

    @property
    def scripts_panel(self):
        return self.app.userInterface.allToolbarPanels.itemById(self.command.PANEL_LOCATION)

    @property
    def command_definition(self):
        return self.app.userInterface.commandDefinitions.itemById(self.command.ID)

    @property
    def command_control(self):
        return self.scripts_panel.controls.itemById(self.command_definition.id) if self.command_definition else None

    def find_command_definition(self):
        return self.command_definition if self.command_definition else self.add_button_definition()

    def add_button_definition(self):
        return self.app.userInterface.commandDefinitions.addButtonDefinition(
                self.command.ID, self.command.DIALOG, self.command.DESC
        )

    def del_command_button(self):
        check_control = self.scripts_panel and self.command_definition
        control = self.command_control if check_control else None

        if self.command_definition:
            self.command_definition.deleteMe()
        if control:
            control.deleteMe()

    def execute(self, inputs):
        with uicontext(self.app, level):
            self.command.execute(inputs)

    def change(self, inputs):
        with uicontext(self.app, level):
            self.command.change(inputs)

    def validate(self, inputs):
        with uicontext(self.app, level):
            return self.command.validate(inputs)

    def create(self, inputs):
        with uicontext(self.app, level):
            self.command.create(inputs)

    def preview(self, inputs):
        with uicontext(self.app, level):
            self.command.preview(inputs)

    def destroy(self, args):
        with uicontext(self.app, level):
            self.command.destroy(args)