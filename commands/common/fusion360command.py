from abc import ABC, abstractmethod

from .uicontext import uicontext


class Fusion360Command(ABC):

    COMMAND_HANDLER = None

    def __init__(self, app):
        self.app = app
        self.handlers = []
        self.created_handler = None

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
        with uicontext(self.app):
            if not self.COMMAND_HANDLER:
                raise RuntimeError

            handler = self.COMMAND_HANDLER(self, self.preview_id)
            self.handlers.append(handler)

            cmd_def = self.find_command_definition()
            if not cmd_def.commandCreated.add(handler):
                raise RuntimeError

    def stop(self):
        self.del_command_button()

    @property
    def scripts_panel(self):
        return self.app.userInterface.allToolbarPanels.itemById(self.scripts_id)

    @property
    def command_definition(self):
        return self.app.userInterface.commandDefinitions.itemById(self.command_id)

    @property
    def command_control(self):
        return self.scripts_panel.controls.itemById(self.command_definition.id) if self.command_definition else None

    def find_command_definition(self):
        return self.command_definition if self.command_definition else self.add_button_definition()

    def add_button_definition(self):
        return self.app.userInterface.commandDefinitions.addButtonDefinition(
                self.command_id, self.command_name, self.command_desc
        )

    def del_command_button(self):
        check_control = self.scripts_panel and self.command_definition
        control = self.command_control if check_control else None

        if self.command_definition:
            self.command_definition.deleteMe()
        if control:
            control.deleteMe()

    def on_execute(self, inputs):
        with uicontext(self.app):
            self._on_execute(inputs)

    def on_change(self, inputs):
        with uicontext(self.app):
            self._on_change(inputs)

    def on_validate(self, inputs):
        with uicontext(self.app):
            self._on_validate(inputs)

    def on_create(self, inputs):
        with uicontext(self.app):
            self._on_create(inputs)

    def on_preview(self, inputs):
        with uicontext(self.app):
            self._on_preview(inputs)

    @property
    @abstractmethod
    def command_id(self):
        pass

    @property
    @abstractmethod
    def scripts_id(self):
        pass

    @property
    @abstractmethod
    def command_desc(self):
        pass

    @property
    @abstractmethod
    def resources(self):
        pass

    @property
    @abstractmethod
    def command_name(self):
        pass

    @property
    @abstractmethod
    def preview_id(self):
        pass

    @abstractmethod
    def _on_execute(self, inputs): pass

    @abstractmethod
    def _on_change(self, inputs): pass

    @abstractmethod
    def _on_validate(self, inputs): pass

    @abstractmethod
    def _on_create(self, inputs): pass

    @abstractmethod
    def _on_preview(self, inputs): pass
