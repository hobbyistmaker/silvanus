from .ecs import ProcessManager
from .ecs import logger
from .entities import Enabled
from .entities import InsideFingers
from .entities import InsidePanel
from .entities import JointPanelOffset
from .entities import KerfJoint
from .entities import Renderable
from .panels import ConfigurePanels
from .render import RenderSystem
from .save import SaveParameters


class Core:

    def __init__(self, dialog):
        self._config = dialog.config
        self._repository = dialog.repository

        self._processes = ProcessManager()
        self._processes._config = self._config
        self._processes._repository = self._repository

    def execute(self, *, orientation, component):
        logger.debug(f'Running execute processes.')
        self._processes.clear()
        self._processes.create_process(SaveParameters)
        self._processes.create_process(RenderSystem, component, orientation, parametric=True)
        self._processes.process()
        self.clear()

    def preview(self, *, orientation, component):
        logger.debug(f'Running preview processes.')
        self._processes.clear()
        self._processes.create_process(RenderSystem, component, orientation)
        self._processes.process()

    def update(self, *, orientation):
        logger.debug(f'Running update processes.')
        self._free_groups()
        self._processes.clear()
        self._processes.create_process(ConfigurePanels, orientation)
        self._processes.process()

    def clear(self):
        self._repository.clear()
        self._processes.clear()

    def _free_groups(self):
        renderables = self._repository.with_components(Renderable).instances

        for renderable in renderables:
            self._repository.remove_entity(renderable.id)

        kerf_joints = self._repository.with_components(KerfJoint).instances
        for joint in kerf_joints:
            self._repository.remove_entity(joint.id)

        panel_joints = self._repository.with_components(KerfJoint).instances
        for joint in panel_joints:
            self._repository.remove_entity(joint.id)

        inside_panels = self._repository.with_components(Enabled, InsidePanel).instances
        for panel in inside_panels:
            self._repository.remove_entity(panel.id)

        inside_fingers = self._repository.with_components(Enabled, InsideFingers).instances
        for joint in inside_fingers:
            self._repository.remove_entity(joint.id)

        enabled = self._repository.with_components(Enabled).instances
        for enable in enabled:
            self._repository.remove_component(enable.id, enable.Enabled)

        self._repository.flush()
