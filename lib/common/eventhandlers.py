import logging

import adsk.core

from .uicontext import uicontext

handlers = []

logger = logging.getLogger('silvanus').getChild(__name__)


class CommandDestroyHandler(adsk.core.CommandEventHandler):

    def __init__(self, command):
        super().__init__()
        self.app = command.app
        self.command = command

    def notify(self, args):
        with uicontext(self.app):
            self.command.on_destroy(args)


class CommandExecuteHandler(adsk.core.CommandEventHandler):

    def __init__(self, command):
        super().__init__()
        self.command = command
        self.app = command.app

    def notify(self, args):
        with uicontext(self.app):
            command = args.firingEvent.sender
            inputs = command.commandInputs

            self.command.on_execute(inputs)


class InputChangedHandler(adsk.core.InputChangedEventHandler):

    def __init__(self, command):
        super().__init__()
        self.command = command
        self.app = command.app

    def notify(self, args):
        with uicontext(self.app):
            cmd_input = args.input

            self.command.on_change(cmd_input)


class ExecutePreviewHandler(adsk.core.CommandEventHandler):

    def __init__(self, command, preview_id):
        super().__init__()
        self.command = command
        self.app = command.app
        self.preview_id = preview_id

    def notify(self, args):
        with uicontext(self.app):
            command = args.firingEvent.sender
            inputs = command.commandInputs

            self.command.on_preview(inputs)


class ValidateInputsHandler(adsk.core.ValidateInputsEventHandler):

    def __init__(self, command):
        super().__init__()
        self.command = command
        self.app = command.app

    def notify(self, args):
        with uicontext(self.app):
            command = args.firingEvent.sender
            inputs = command.commandInputs

            args.areInputsValid = self.command.on_validate(inputs)


class CreatedEventHandler(adsk.core.CommandCreatedEventHandler):

    def __init__(self, command, preview_id):
        super().__init__()
        self.command = command
        self.app = self.command.app
        self.preview_id = preview_id

    def notify(self, args):
        global handlers

        with uicontext(self.app):
            command = args.command

            on_execute = CommandExecuteHandler(self.command)
            command.execute.add(on_execute)
            handlers.append(on_execute)

            on_preview = ExecutePreviewHandler(self.command, self.preview_id)
            command.executePreview.add(on_preview)
            handlers.append(on_preview)

            on_change = InputChangedHandler(self.command)
            command.inputChanged.add(on_change)
            handlers.append(on_change)

            on_validate = ValidateInputsHandler(self.command)
            command.validateInputs.add(on_validate)
            handlers.append(on_validate)

            on_destroy = CommandDestroyHandler(self.command)
            command.destroy.add(on_destroy)
            handlers.append(on_destroy)

            self.command.on_create(command)
