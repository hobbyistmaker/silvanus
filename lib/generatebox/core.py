import logging
import math
from collections import defaultdict
from collections import namedtuple
from pprint import pformat

import adsk.core
import adsk.fusion

from .entities import ActualFingerCount
from .entities import ActualFingerWidth
from .entities import AxisFlag
from .entities import AxisFingerType
from .entities import ConfigItem
from .entities import DefaultFingers
from .entities import EnableInput
from .entities import Enabled
from .entities import EstimatedFingers
from .entities import ExtrusionDistance
from .entities import JointAxis
from .entities import JointItem
from .entities import FingerOffset
from .entities import FingerPatternDistance
from .entities import FingerType
from .entities import FingerWidth
from .entities import FingerWidthInput
from .entities import GroupItem
from .entities import GroupName
from .entities import GroupOrientation
from .entities import GroupPanel
from .entities import GroupPanels
from .entities import GroupPlaneSelector
from .entities import GroupProfile
from .entities import GroupTransform
from .entities import Height
from .entities import HeightInput
from .entities import HeightOrientation
from .entities import Inputs
from .entities import Kerf
from .entities import KerfInput
from .entities import Length
from .entities import LengthInput
from .entities import LengthOrientation
from .entities import MaxOffset
from .entities import MaxOffsetInput
from .entities import PanelGroup
from .entities import PanelName
from .entities import PanelOffset
from .entities import PanelOrientation
from .entities import PanelProfile
from .entities import PanelEndReferencePoint
from .entities import PanelStartReferencePoint
from .entities import PanelSubGroups
from .entities import Parameter
from .entities import ParentOrientation
from .entities import ParentPanel
from .entities import ReferencePointInput
from .entities import Renderable
from .entities import StartPanelOffset
from .entities import Thickness
from .entities import ThicknessInput
from .entities import Width
from .entities import WidthInput
from .entities import WidthOrientation
from .fusion import FaceProfile
from .fusion import FingerCutsPattern
from .fusion import PanelFingerSketch
from .fusion import PanelProfileSketch

# noinspection SpellCheckingInspection
logger = logging.getLogger('silvanus.lib.generatebox.core')


class ComponentQuery:

    def __init__(self, components):
        self._components = components

    def for_each(self, func):
        for component in self._components:
            func(component)

    def with_all(self, func):
        func(self._components)

    def with_true(self, filter_fun):
        return ComponentQuery(filter(filter_fun, self._components))

    @property
    def instances(self):
        return self._components


class Entity:
    em = None

    def __init__(self, id_):
        self.id = id_

    def add_component(self, component):
        self.em.add_component(self.id, component)
        return self

    def add_components(self, *components):
        self.em.add_components(self.id, *components)
        return self

    def remove_component(self, component):
        self.em.remove_component(self.id, component)
        return self


class Repository:
    axes = defaultdict(lambda: defaultdict(lambda: defaultdict(dict)))
    names = defaultdict(lambda: defaultdict(lambda: defaultdict(list)))
    input_panels = { }
    defined_panels = { }
    thickness_groups = defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: defaultdict(list))))
    parameters = { }
    enabled = { }
    _entity_remove_buffer = []

    enable = { }
    override = { }
    thickness = { }
    dimensions = { }
    axis = { }

    _components = defaultdict(dict)
    _entities = defaultdict(dict)

    orientation = 0
    root = None
    preview = False
    defined = False

    _next_entity_id = 0

    def create(self, *components):
        self._next_entity_id += 1
        entity = self._next_entity_id

        for component in components:
            component_type = type(component)
            self._components[component_type][entity] = component
            self._entities[entity][component_type] = component

        new_entity = Entity(entity)
        new_entity.em = self
        return new_entity

    def add_component(self, entity, component):
        component_type = type(component)
        self._components[component_type][entity] = component
        self._entities[entity][component_type] = component

    def add_components(self, entity, *components):
        for component in components:
            component_type = type(component)
            self._components[component_type][entity] = component
            self._entities[entity][component_type] = component

    def with_components(self, *component_types):
        all_components = self._components
        all_entities = self._entities

        component_names = [
            component_type.__name__
            for component_type in component_types
        ]

        EntityQuery = namedtuple('EntityQuery', ['id', *component_names])

        class EntityResult(EntityQuery):
            em = self

            def add_component(s, component):
                s.em.add_component(s.id, component)
                return s

            def add_components(s, *components):
                s.em.add_components(s.id, *components)
                return s

            def remove_component(s, component):
                s.em.remove_component(s.id, component)
                return s

        components = [
            EntityResult(entity, *[all_entities[entity][component_type] for component_type in component_types])
            for entity in set.intersection(*[
                set(all_components[component_type])
                for component_type in component_types
            ])
        ]
        return ComponentQuery(components)

    def remove_entity(self, entity):
        self._entity_remove_buffer.append(entity)

    def remove_component(self, entity, component_type):
        if component_type not in self._components:
            return False

        del self._components[component_type][entity]
        if not self._components[component_type]:
            del self._components[component_type]

        if not self._entities[entity]:
            del self._entities[entity]

        return True

    def _remove_entity_immediate(self, entity):
        self._entities[entity].clear()
        del self._entities[entity]

    def flush(self):
        for entity in self._entity_remove_buffer:
            for component_type in self._entities[entity]:
                self.remove_component(entity, component_type)

            self._remove_entity_immediate(entity)

        self._entity_remove_buffer.clear()

    def clear(self):
        self.axes.clear()
        self.names.clear()
        self.thickness_groups.clear()
        self.enabled.clear()
        self.preview = False
        self.root = None
        self.orientation = 0
        self.parameters.clear()
        self.defined_panels.clear()
        self._next_entity_id = 0
        self._components.clear()
        self._entities.clear()


class Process:
    priority = 0
    _config = None
    _repository = None


class ProcessManager(Process):

    def __init__(self):
        self._processes = []

        self._post_init_()

    def _post_init_(self):
        pass

    def create_process(self, process, *args, **kwargs):
        instance = process(*args, **kwargs)
        self._processes.append(instance)
        return instance

    def add_process(self, process, priority=0):
        self.create_process(process, priority)
        return self

    def process(self, *args, **kwargs):
        self._process(*args, **kwargs)

    def _process(self, *args, **kwargs):
        for process in self._processes:
            process._config = self._config
            process._repository = self._repository
            process.process(*args, **kwargs)
            self._repository.flush()

    def clear(self):
        self._processes.clear()


class Core:

    def __init__(self, dialog):
        self._config = dialog.config
        self._repository = dialog.repository

        self._processes = ProcessManager()

    def execute(self, *, panels, orientation, component):
        self._repository.input_panels = panels
        self._repository.orientation = orientation
        self._repository.root = component

        self._processes.create_process(ConfigurePanels, orientation)
        self._processes.create_process(RenderSystem, component, orientation)
        self._processes.create_process(SaveParameters)

        self._processes._config = self._config
        self._processes._repository = self._repository
        self._processes.process()
        self.clear()

    def preview(self, *, panels, orientation, component):
        self._repository.input_panels = panels
        self._repository.orientation = orientation
        self._repository.root = component
        self._repository.preview = True

        self._processes.create_process(ConfigurePanels, orientation)
        self._processes.create_process(RenderSystem, component, orientation)

        logger.debug(f'{pformat(self._repository._components)}')

        self._processes._config = self._config
        self._processes._repository = self._repository
        self._processes.process()

        self._processes.clear()

    def clear(self):
        self._repository.clear()
        self._processes.clear()


class ConfigurePanels(Process):

    def __init__(self, orientation):
        self._orientation = orientation

    def process(self):
        self._configure_parameters()

        self._disable_panels()
        self._enable_panels()

        self._add_thickness_inputs()
        self._add_length_inputs()
        self._add_width_inputs()
        self._add_height_inputs()
        self._add_kerf_inputs()
        self._add_finger_width_inputs()
        self._add_max_offset_inputs()
        self._add_extrusion_distance()

        self._add_reference_points()

        self._add_orientations()
        self._add_height_orientation()
        self._add_length_orientation()
        self._add_width_orientation()
        self._add_height_panel_start_points()
        self._add_length_panel_start_points()
        self._add_width_panel_start_points()

        self._adjust_start_panel_offset()

        self._kerf_adjust_profiles()

        self._define_finger_joints()

    def _configure_parameters(self):
        self._repository.parameters = {
            key: {
                ConfigItem.Name:    parameter[ConfigItem.Name],
                ConfigItem.Control: self._config.controls[key],
                ConfigItem.Enabled: parameter[ConfigItem.Enabled]
            }
            for key, parameter in self._config.parameters.items()
        }

        overrides = {
            key: self._config.controls[value]
            for key, value in self._config.overrides.items()
        }
        for key, parameter in filter(lambda o: not o[1].value, overrides.items()):
            self._repository.parameters[key][ConfigItem.Control] = self._config.controls[Inputs.Thickness]
            self._repository.parameters[key][ConfigItem.Name] = 'thickness'

        enabled = {
            key: self._config.controls[value]
            for key, value in self._config.enabled.items()
        }
        for key, parameter in filter(lambda o: not o[1].value, enabled.items()):
            self._repository.parameters[key][ConfigItem.Enabled] = False

    def _read_control(self, source, dest):
        control = self._config.controls[source.control]
        return dest(control.value, control.name, control.unitType)

    def _enable_panels(self):
        def read_enabled(entity):
            return entity.EnableInput.control.value

        self._repository.with_components(EnableInput).with_true(read_enabled).for_each(
                lambda c: c.add_component(Enabled(True))
        )

    def _disable_panels(self):
        def read_enabled(entity):
            return not entity.EnableInput.control.value

        self._repository.with_components(EnableInput, Enabled).with_true(read_enabled).for_each(
                lambda c: c.remove_component(Enabled)
        )

    def _add_thickness_inputs(self):
        self._repository.with_components(ThicknessInput).for_each(
                lambda c: c.add_component(self._read_control(c.ThicknessInput, Thickness))
        )

    def _add_length_inputs(self):
        self._repository.with_components(LengthInput).for_each(
                lambda c: c.add_component(self._read_control(c.LengthInput, Length))
        )

    def _add_width_inputs(self):
        self._repository.with_components(WidthInput).for_each(
                lambda c: c.add_component(self._read_control(c.WidthInput, Width))
        )

    def _add_height_inputs(self):
        self._repository.with_components(HeightInput).for_each(
                lambda c: c.add_component(self._read_control(c.HeightInput, Height))
        )

    def _add_extrusion_distance(self):
        self._repository.with_components(Thickness).for_each(
                lambda c: c.add_component(
                        ExtrusionDistance(c.Thickness.value, c.Thickness.expression, c.Thickness.unitType)
                )
        )

    def _add_kerf_inputs(self):
        def read_kerf(entity):
            control = self._config.controls[entity.KerfInput.control]
            return bool(control.value)

        self._repository.with_components(KerfInput).with_true(read_kerf).for_each(
                lambda c: c.add_component(self._read_control(c.KerfInput, Kerf))
        )

    def _add_finger_width_inputs(self):
        self._repository.with_components(FingerWidthInput).for_each(
                lambda c: c.add_component(self._read_control(c.FingerWidthInput, FingerWidth))
        )

    def _add_max_offset_inputs(self):
        def read_max_offset(entities):
            for entity in entities:
                control = self._config.controls[entity.MaxOffsetInput.control]
                entity.add_component(MaxOffset(control.value, control.name, control.unitType))

        self._repository.with_components(MaxOffsetInput).with_all(read_max_offset)

    def _add_orientations(self):
        selector = {
            AxisFlag.Height: HeightOrientation,
            AxisFlag.Width:  WidthOrientation,
            AxisFlag.Length: LengthOrientation
        }
        self._repository.with_components(PanelOrientation).for_each(
                lambda c: c.add_component(selector[c.PanelOrientation.axis]())
        )

    def _add_height_orientation(self):
        self._repository.with_components(Enabled, HeightOrientation, Length, Width, Height, Thickness).for_each(
                lambda c: c.add_components(PanelProfile(c.Length, c.Width))
        )

    def _add_length_orientation(self):
        self._repository.with_components(Enabled, LengthOrientation, Length, Width, Height, Thickness).for_each(
                lambda c: c.add_components(PanelProfile(c.Width, c.Height))
        )

    def _add_width_orientation(self):
        self._repository.with_components(Enabled, WidthOrientation, Length, Width, Height, Thickness).for_each(
                lambda c: c.add_components(PanelProfile(c.Length, c.Height))
        )

    def _add_height_panel_start_points(self):
        def add_start_point(entity):
            end = entity.PanelEndReferencePoint
            thickness = entity.Thickness

            expression = {
                False: '0',
                True: f'{end.height.expression} - {thickness.expression}'
            }
            height_value = end.height.value - thickness.value
            start_height = Height(
                height_value,
                expression[bool(height_value)],
                end.height.unitType
            )
            entity.add_components(PanelStartReferencePoint(
                    Length(0, '', end.length.unitType),
                    Width(0, '', end.width.unitType),
                    height_value
                ), PanelOffset(start_height.value, start_height.expression, start_height.unitType)
            )
        self._repository.with_components(HeightOrientation, PanelEndReferencePoint, Thickness).for_each(add_start_point)

    def _add_length_panel_start_points(self):
        def add_start_point(entity):
            end = entity.PanelEndReferencePoint
            thickness = entity.Thickness

            expression = {
                False: '0',
                True: f'{end.length.expression} - {thickness.expression}'
            }

            length_value = end.length.value - thickness.value
            start_length = Length(
                length_value,
                expression[bool(length_value)],
                end.length.unitType
            )

            entity.add_components(PanelStartReferencePoint(
                    start_length,
                    Width(0, '', end.width.unitType),
                    Height(0, '', end.height.unitType)
                ), PanelOffset(start_length.value, start_length.expression, start_length.unitType)
            )
        self._repository.with_components(LengthOrientation, PanelEndReferencePoint, Thickness).for_each(add_start_point)

    def _add_width_panel_start_points(self):
        def add_start_point(entity):
            end = entity.PanelEndReferencePoint
            thickness = entity.Thickness

            expression = {
                False: '0',
                True: f'{end.width.expression} - {thickness.expression}'
            }

            width_value = end.width.value - thickness.value
            start_width = Width(
                    width_value,
                    expression[bool(width_value)],
                    end.width.unitType
            )

            entity.add_components(PanelStartReferencePoint(
                    Length(0, '', end.length.unitType),
                    start_width,
                    Height(0, '', end.width.unitType)
                ),
                PanelOffset(start_width.value, start_width.expression, start_width.unitType)
            )
        self._repository.with_components(WidthOrientation, PanelEndReferencePoint, Thickness).for_each(add_start_point)

    def _add_reference_points(self):
        def add_point(entities):
            for entity in entities:
                length = self._config.controls[entity.ReferencePointInput.length]
                width = self._config.controls[entity.ReferencePointInput.width]
                height = self._config.controls[entity.ReferencePointInput.height]

                end_length = Length(length.value, length.name, length.unitType)
                end_width = Width(width.value, width.name, width.unitType)
                end_height = Height(height.value, height.name, height.unitType)

                entity.add_component(
                        PanelEndReferencePoint(
                            end_length,
                            end_width,
                            end_height
                        )
                )

        self._repository.with_components(ReferencePointInput).with_all(add_point)

    def _kerf_adjust_profiles(self):
        def add_kerf(component):
            profile, kerf = component.PanelProfile, component.Kerf
            return PanelProfile(
                    Length(
                            profile.length.value + kerf.value,
                            f'{profile.length.expression} + {kerf.expression}',
                            profile.length.unitType
                    ),
                    Width(
                            profile.width.value + kerf.value,
                            f'{profile.width.expression} + {kerf.expression}',
                            profile.width.unitType
                    )
            )

        self._repository.with_components(PanelProfile, Kerf).for_each(
                lambda c: c.add_component(add_kerf(c))
        )

    def _adjust_start_panel_offset(self):
        if self._orientation:
            return

        def adjust_extrusion_offset(entity):
            selector = {
                False: '',
                True: entity.MaxOffset.expression
            }

            new_value = entity.PanelOffset.value * -1 + entity.MaxOffset.value
            entity.add_component(
                    PanelOffset(
                            new_value,
                            selector[bool(new_value)],
                            entity.MaxOffset.unitType
                    ))

    def _define_finger_joints(self):
        joint_orientations = defaultdict(lambda: {
            JointItem.ParentPanels: [],
            JointItem.JointPanels:  []
        })

        finger_joint_instances = self._repository.with_components(
                Enabled, JointAxis, AxisFingerType, ParentPanel, ParentOrientation, PanelName
        ).instances

        for instance in finger_joint_instances:
            joint = joint_orientations[instance.JointAxis.orientation]
            joint[JointItem.ParentPanels].append(instance)

        root_panels = self._repository.with_components(
                Enabled, PanelOrientation, PanelOffset, Length, Width, Height, PanelName, Thickness
        ).instances

        for instance in root_panels:
            joint = joint_orientations[instance.PanelOrientation.axis]
            joint[JointItem.JointPanels].append(instance)

        distance_selector = {
            (AxisFlag.Width, AxisFlag.Height):  lambda c: c.Length,
            (AxisFlag.Width, AxisFlag.Length):  lambda c: c.Height,
            (AxisFlag.Height, AxisFlag.Length): lambda c: c.Width,
            (AxisFlag.Height, AxisFlag.Width):  lambda c: c.Length,
            (AxisFlag.Length, AxisFlag.Width):  lambda c: c.Height,
            (AxisFlag.Length, AxisFlag.Height): lambda c: c.Width
        }

        for joint_axis, joint_data in joint_orientations.items():
            parent_panels = joint_data[JointItem.ParentPanels]
            joint_panels = joint_data[JointItem.JointPanels]
            for parent_panel in parent_panels:
                for joint_panel in joint_panels:
                    total_distance = distance_selector[(parent_panel.JointAxis.orientation,
                                                        parent_panel.ParentOrientation.value)](joint_panel)
                    logger.debug(f'Join {parent_panel.PanelName.value} with {joint_panel.PanelName.value} along {total_distance.expression} with {parent_panel.AxisFingerType.finger_type} joint of {joint_panel.Thickness.expression}.')

        logger.debug(f'Finger Joints Joined: {pformat(joint_orientations, width=160)}')


class RenderSystem(Process):

    def __init__(self, root, orientation):
        self._root = root
        self._orientation = orientation

    def process(self):
        self._define_groups()
        self._render_profile_groups()

        self._free_groups()

    def _define_groups(self):
        def group_data(): return {
            GroupItem.Names:       [],
            GroupItem.Thicknesses: defaultdict(list)
        }

        def profile_group(): return defaultdict(group_data)

        groups = defaultdict(profile_group)

        self._define_group_profiles(groups)
        self._add_group_entities(groups)

    def _add_group_entities(self, groups):
        for axis, profiles in groups.items():
            for profile, config in profiles.items():
                self._repository.create(
                        GroupName('-'.join(config[GroupItem.Names])),
                        GroupOrientation(axis),
                        GroupProfile(*profile),
                        GroupTransform(config[GroupItem.Transform]),
                        GroupPlaneSelector(config[GroupItem.PlaneSelector]),
                        PanelSubGroups([
                            GroupPanels(thickness, values)
                            for thickness, values in config[GroupItem.Thicknesses].items()
                        ])
                )

    def _define_group_profiles(self, groups):
        entities = self._repository.with_components(
                Enabled, Renderable, PanelProfile, PanelOrientation, PanelName, Thickness, PanelOffset
        ).instances

        for entity in entities:
            group = groups[entity.PanelOrientation.axis][
                (entity.PanelProfile.length, entity.PanelProfile.width)
            ]
            group[GroupItem.Names].append(entity.PanelName.value)
            group[GroupItem.Transform] = self._select_group_transform(entity.PanelOrientation)
            group[GroupItem.PlaneSelector] = self._select_group_plane(entity.PanelOrientation)
            group[GroupItem.Thicknesses][entity.Thickness].append(GroupPanel(
                entity.PanelName.value,
                entity.PanelOffset.value,
                entity.PanelOffset.expression,
                entity.PanelOffset.unitType)
            )

        logger.debug(f'len({len(groups)}: {pformat(groups)})')

    def _select_group_transform(self, orientation):
        transform = {
            0: {
                AxisFlag.Height: lambda x, y, z: (x, -y, z),
                AxisFlag.Length: lambda x, y, z: (-x, y, z),
                AxisFlag.Width:  lambda x, y, z: (x, y, z)
            },
            1: {
                AxisFlag.Height: lambda x, y, z: (x, y, z),
                AxisFlag.Length: lambda x, y, z: (x, y, z),
                AxisFlag.Width:  lambda x, y, z: (x, y, z)
            }
        }

        return transform[self._orientation][orientation.axis]

    def _select_group_plane(self, orientation):
        plane = {
            0: {
                AxisFlag.Height: self._root.xZConstructionPlane,
                AxisFlag.Length: self._root.yZConstructionPlane,
                AxisFlag.Width:  self._root.xYConstructionPlane
            },
            1: {
                AxisFlag.Height: self._root.xYConstructionPlane,
                AxisFlag.Length: self._root.yZConstructionPlane,
                AxisFlag.Width:  self._root.xZConstructionPlane
            }
        }

        return plane[self._orientation][orientation.axis]

    def _free_groups(self):
        self._repository.with_components(GroupProfile).for_each(
                lambda e: self._repository.remove_entity(e.id)
        )

    def _render_profile_groups(self):
        self._repository.with_components(
                GroupName, GroupProfile, GroupTransform, GroupPlaneSelector, PanelSubGroups
        ).for_each(self._render_profile_group)

    def _render_profile_group(self, entity):
        end = (entity.GroupProfile.length.value, entity.GroupProfile.width.value)
        name = entity.GroupName.value

        app = adsk.core.Application.get()
        timeline = app.activeProduct.timeline
        start = timeline.markerPosition

        with PanelProfileSketch(
                plane_selector=entity.GroupPlaneSelector.call,
                transform=entity.GroupTransform.call,
                end=end,
                name=name
        ) as profile_sketch:
            for subgroup in entity.PanelSubGroups.groups:
                self._extrude_profiles(profile_sketch, subgroup)

            self._repository.remove_entity(entity.id)

        end = timeline.markerPosition

        if (end - 1) - start <= 1:
            return

        group = timeline.timelineGroups.add(start, end - 1)
        group.name = f'{name} Panel Group'

    def _extrude_profiles(self, profile, subgroup):
        _render_func = {
            False: self._render_single_extrusion,
            True:  self._render_copy_extrusion
        }
        current_reference = profile

        for id_, panel in enumerate(sorted(subgroup.panels, key=lambda p: p.value)):
            func = _render_func[bool(id_)]
            # noinspection PyArgumentList
            current_reference = func(current_reference, subgroup.thickness, panel)

    def _render_single_extrusion(self, profile, thickness, panel):
        extrusion = profile.extrude(panel, thickness)
        extrusion.name = f'{panel.name} Panel Extrusion'
        body = extrusion.bodies.item(0)
        body.name = f'{panel.name} Panel Body'

        # if not self._repository.preview:
        #     self._render_finger_cuts(root, extrusion, panel)

        return extrusion

    @staticmethod
    def _render_copy_extrusion(reference, thickness, panel):
        with FaceProfile(reference, name='') as profile:
            extrusion = profile.extrude_reverse(panel, thickness, -1)
            extrusion.name = f'{panel.name} Panel Extrusion'
            body = extrusion.bodies.item(0)
            body.name = f'{panel.name} Panel Body'

        return reference


class HoldEmHere:

    def _configure_panels_and_faces(self):
        for name, panel_info in self._repository.defined_panels.items():
            panel = panel_info[ConfigItem.Panel]
            data = panel_info[ConfigItem.PanelData]
            profile = panel_info[ConfigItem.Profile]
            faces_config = data[ConfigItem.Faces]
            panel.faces = self._configure_panel_faces(panel, faces_config)

    def _configure_panel_faces(self, panel, faces_config):
        faces_repository = defaultdict(
                lambda: defaultdict(
                        lambda: defaultdict(
                                lambda: defaultdict(
                                        lambda: defaultdict(
                                                lambda: defaultdict(list)
                                        )))))

        finger_selector = {
            FingerType.Normal:  self._configure_normal_fingers,
            FingerType.Inverse: self._configure_inverse_fingers
        }

        parameters = {
            ConfigItem.Length:    panel.length,
            ConfigItem.Width:     panel.width,
            ConfigItem.Height:    panel.height,
            ConfigItem.Thickness: panel.thickness
        }

        for face, face_data in faces_config.items():
            enabled = self._config.controls[face_data[ConfigItem.Enabled]].value
            axis = face_data[ConfigItem.Axis]

            kerf = panel.kerf

            length = parameters[face_data[ConfigItem.Length]]
            width = parameters[face_data[ConfigItem.Width]]
            profile = self._profile(length, width, kerf)
            finger_type = face_data[ConfigItem.FingerType]

            jointed_panel = self._repository.input_panels[face_data[ConfigItem.Joint]]
            panel_offset = jointed_panel.offset
            depth = jointed_panel.thickness

            axes = face_data[ConfigItem.FingerAxis]

            face_config = {
                ConfigItem.Face:        face,
                ConfigItem.Kerf:        kerf,
                ConfigItem.Length:      length,
                ConfigItem.Offset:      panel_offset,
                ConfigItem.Width:       width,
                ConfigItem.FingerAxis:  axes,
                ConfigItem.FingerDepth: depth
            }

            finger_data = {
                ConfigItem.Fingers: finger_selector[finger_type](length, panel.finger_width, panel.kerf),
                ConfigItem.Face:    face_config
            }

            faces_repository[enabled][axis][profile][axes][finger_type][depth.expression].append(finger_data)

        return faces_repository[True]

    @staticmethod
    def _configure_normal_fingers(length, finger_width, kerf):
        default_fingers = DefaultFingers(
                math.ceil(length.value / finger_width.value),
                f'ceil({length.expression} / {finger_width.expression})'
        )
        estimated_fingers = EstimatedFingers(
                max(3, (math.floor(default_fingers.value / 2) * 2) - 1),
                f'max(3; (floor({default_fingers.expression} / 2) * 2) - 1)'
        )

        desired_finger_width = length.value / estimated_fingers.value
        afw_kerf_selector = {
            True:  f'({length.expression} / {estimated_fingers.expression}) - {kerf.expression}',
            False: f'{length.expression} / {estimated_fingers.expression}'
        }
        actual_finger_width = ActualFingerWidth(
                (length.value / estimated_fingers.value) - kerf.value,
                afw_kerf_selector[bool(kerf.value)]
        )

        fo_kerf_selector = {
            True:  f'({actual_finger_width.expression} + ({kerf.expression} * 2))',
            False: f'{actual_finger_width.expression}'
        }
        finger_offset = FingerOffset(
                actual_finger_width.value + (kerf.value * 2),
                fo_kerf_selector[bool(kerf.value)]
        )

        actual_number_fingers = ActualFingerCount(
                math.floor(estimated_fingers.value / 2),
                f'floor({estimated_fingers.expression} / 2)'
        )
        distance = FingerPatternDistance(
                (estimated_fingers.value - 3) * desired_finger_width,
                f'({estimated_fingers.expression} - 3) * ({afw_kerf_selector[bool(kerf.value)]})'
        )

        return {
            ConfigItem.FingerType:            FingerType.Normal,
            ConfigItem.FingerOffset:          finger_offset,
            ConfigItem.FingerCount:           actual_number_fingers,
            ConfigItem.FingerWidth:           actual_finger_width,
            ConfigItem.FingerPatternDistance: distance,
        }

    @staticmethod
    def _configure_inverse_fingers(length, finger_width, kerf):
        default_fingers = DefaultFingers(
                math.ceil(length.value / finger_width.value),
                f'ceil({length.expression} / {finger_width.expression})'
        )
        estimated_fingers = EstimatedFingers(
                max(3, (math.floor(default_fingers.value / 2) * 2) - 1),
                f'max(3; (floor({default_fingers.expression} / 2) * 2) - 1)'
        )

        desired_finger_width = length.value / estimated_fingers.value

        afw_kerf_selector = {
            True:  f'({length.expression} / {estimated_fingers.expression}) - {kerf.expression}',
            False: f'{length.expression} / {estimated_fingers.expression}'
        }
        actual_finger_width = ActualFingerWidth(
                (length.value / estimated_fingers.value) - kerf.value,
                afw_kerf_selector[bool(kerf.value)]
        )

        fo_kerf_selector = {
            True:  f'({kerf.expression})',
            False: ''
        }
        finger_offset = FingerOffset(
                0 + kerf.value,
                fo_kerf_selector[bool(kerf.value)]
        )

        actual_number_fingers = ActualFingerCount(
                math.ceil(estimated_fingers.value / 2),
                f'ceil({estimated_fingers.expression} / 2)'
        )
        distance = FingerPatternDistance(
                (estimated_fingers.value - 1) * desired_finger_width,
                f'({estimated_fingers.expression} - 1) * ({afw_kerf_selector[bool(kerf.value)]})'
        )

        return {
            ConfigItem.FingerType:            FingerType.Inverse,
            ConfigItem.FingerOffset:          finger_offset,
            ConfigItem.FingerCount:           actual_number_fingers,
            ConfigItem.FingerWidth:           actual_finger_width,
            ConfigItem.FingerPatternDistance: distance,
        }

    def _separate_thickness_groups(self):
        for panel, profile in (
                (panel[ConfigItem.Panel], panel[ConfigItem.Profile])
                for panel in self._repository.defined_panels.values()
        ):
            enabled, axis = panel.enabled, panel.axis
            self._repository.thickness_groups[enabled][axis][profile][panel.thickness.expression].append(panel)

    def _dimension(self, data, key, wrapper):
        return wrapper(*self._control_parameter(data[key]))

    def _control_parameter(self, key):
        control = self._config.controls[key]
        return control.value, self._config.parameters[key][ConfigItem.Name], control.unitType

    def _control_to_parameter(self, control):
        return control.value, control.name, control.unitType

    def _offset(self, data, key, wrapper, orientation, kerf):
        offset = self._dimension(data[key], orientation, wrapper)
        return wrapper(offset.value + kerf.value, f'{offset.expression} + {kerf.expression}')

    def _profile(self, length, width, kerf):
        profile_length = Length(*self._control_to_parameter(length))
        profile_width = Width(*self._control_to_parameter(width))

        return PanelProfile(
                Length(
                        profile_length.value + kerf.value,
                        ' + '.join([val.expression for val in filter(lambda s: s.value, [profile_length, kerf])]),
                        profile_length.unitType
                ),
                Width(
                        profile_width.value + kerf.value,
                        ' + '.join([val.expression for val in filter(lambda s: s.value, [profile_width, kerf])]),
                        profile_width.unitType
                )
        )

    def _thickness(self, data, override, key):
        selector = {
            True:  lambda d, k: self._dimension(d, k, Thickness),
            False: lambda d, k: Thickness(*self._control_parameter(Inputs.Thickness))
        }
        return selector[override.value](data, key)


class RenderPanels(ProcessManager):

    def process(self):
        root = self._repository.root

        for axis_key, axis in self._repository.enabled.items():
            for profile in axis.items():
                self._render_profiles(root, profile)

    def _render_profiles(self, root, profile):
        app = adsk.core.Application.get()
        timeline = app.activeProduct.timeline

        profile_key, profile_data = profile

        name = profile_data[ConfigItem.ProfileName]
        selector = profile_data[ConfigItem.PlaneSelector]
        transform = profile_data[ConfigItem.ProfileTransform]
        dimensions = profile_data[ConfigItem.Profile]

        start = timeline.markerPosition

        with PanelProfileSketch(
                plane_selector=selector(root), transform=transform,
                end=(dimensions.length.value, dimensions.width.value), name=name
        ) as profile_sketch:
            for group, panels in profile_data[ConfigItem.ThicknessGroups].items():
                self._extrude_profiles(root, profile_sketch, group, panels)

        end = timeline.markerPosition

        group = timeline.timelineGroups.add(start, end - 1)
        group.name = f'{name} Panel Group'

    def _extrude_profiles(self, root, profile, group, panels):
        _render_func = {
            False: self._render_single_extrusion,
            True:  self._render_copy_extrusion
        }
        current_reference = profile

        for id_, panel in enumerate(sorted(panels, key=lambda p: p.offset.value)):
            func = _render_func[bool(id_)]
            # noinspection PyArgumentList
            current_reference = func(root, current_reference, panel)

    def _render_single_extrusion(self, root, profile, panel):
        extrusion = profile.extrude(panel.offset, panel.thickness, panel.direction)
        extrusion.name = f'{panel.name} Panel Extrusion'
        body = extrusion.bodies.item(0)
        body.name = f'{panel.name} Panel Body'

        if not self._repository.preview:
            self._render_finger_cuts(root, extrusion, panel)

        return extrusion

    @staticmethod
    def _render_copy_extrusion(_, reference, panel):
        with FaceProfile(reference, name='') as profile:
            extrusion = profile.extrude(panel.offset, panel.thickness, panel.direction)
            extrusion.name = f'{panel.name} Panel Extrusion'
            body = extrusion.bodies.item(0)
            body.name = f'{panel.name} Panel Body'

        return reference

    def _render_finger_cuts(self, root, extrusion, panel):
        cuts_to_make, kerf_cuts_to_make = self._render_finger_profiles(panel, extrusion)
        for axes, cut_config in kerf_cuts_to_make.items():
            cuts = []

            for sketch, face, finger in cut_config[ConfigItem.FingerCuts]:
                cut = self._render_finger_cut(sketch, face, panel.kerf)
                cut.name = f'{sketch.base_name} Extrusion'
                cuts.append(cut)

            axes = cut_config[ConfigItem.FingerAxis]
            distance = cut_config[ConfigItem.FingerPatternDistance]
            count = cut_config[ConfigItem.FingerCount]
            orientation = cut_config[ConfigItem.Orientation]

            replicator = FingerCutsPattern(root, orientation)
            pattern = replicator.copy(axes=axes, cuts=cuts, distance=distance, count=count)
            pattern.name = f'{sketch}'

        for axes, cut_config in cuts_to_make.items():
            cuts = []

            for sketch, face, finger in cut_config[ConfigItem.FingerCuts]:
                cut = self._render_finger_cut(sketch, face, panel.kerf)
                cut.name = f'{sketch.base_name} Finger Extrusion'
                cuts.append(cut)

            axes = cut_config[ConfigItem.FingerAxis]
            distance = cut_config[ConfigItem.FingerPatternDistance]
            count = cut_config[ConfigItem.FingerCount]
            orientation = cut_config[ConfigItem.Orientation]

            replicator = FingerCutsPattern(root, orientation)
            replicator.copy(axes=axes, cuts=cuts, distance=distance, count=count)

    def _render_finger_profiles(self, panel, extrusion):
        face_selectors = panel.face_selectors
        finger_config = self._configure_fingers(panel)

        cuts_config = defaultdict(lambda: {
            ConfigItem.FingerAxis:            (AxisFlag.Length, AxisFlag.Width),
            ConfigItem.FingerPatternDistance: 0,
            ConfigItem.FingerCount:           0,
            ConfigItem.FingerCuts:            []
        })
        kerf_config = defaultdict(lambda: {
            ConfigItem.FingerAxis:            (AxisFlag.Length, AxisFlag.Width),
            ConfigItem.FingerPatternDistance: 0,
            ConfigItem.FingerCount:           0,
            ConfigItem.FingerCuts:            []
        })

        finger_data = (
            (axis, profile, axes, finger_type, finger_type_config)
            for axis, axes_data in finger_config.items()
            for profile, profile_data in axes_data.items()
            for axes, finger_type_data in profile_data.items()
            for finger_type, finger_type_config in finger_type_data.items()
        )

        for axis, profile, axes, finger_type, finger_type_config in finger_data:
            faces_data = finger_type_config[ConfigItem.Faces]
            finger_data = finger_type_config[ConfigItem.Fingers]

            kerf = faces_data[0][ConfigItem.Kerf]

            offset = finger_data[ConfigItem.FingerOffset]
            finger_width = finger_data[ConfigItem.FingerWidth]
            distance = finger_data[ConfigItem.FingerPatternDistance]
            count = finger_data[ConfigItem.FingerCount]

            sorted_faces = sorted(faces_data, key=lambda f: f[ConfigItem.Offset].value)
            names = '-'.join([item[ConfigItem.Face].name for item in sorted_faces])

            logger.debug(f'Sorted Faces: {sorted_faces}')
            first_face = sorted_faces[0]
            selected_face = first_face[ConfigItem.Face]
            first_offset = first_face[ConfigItem.Offset]

            logger.debug(f'First face: {sorted_faces[0]}')
            if kerf.value and finger_type == FingerType.Inverse:
                length = faces_data[0][ConfigItem.Length]
                with PanelFingerSketch(
                        extrusion=extrusion,
                        selector=face_selectors[selected_face],
                        name=f'{panel.name} {names} Kerf',
                        start=Offset(0, '', 'cm'),
                        end=(kerf, panel.thickness.value),
                        transform=panel.profile_transform,
                        orientation=panel.orientation
                ) as kerf_sketch:
                    for face in sorted_faces:
                        offset = face[ConfigItem.Offset]
                        kerf_config[axes].update({
                            ConfigItem.FingerAxis:            axes,
                            ConfigItem.FingerPatternDistance: length,
                            ConfigItem.FingerCount:           ActualFingerCount(2, '2'),
                            ConfigItem.Orientation:           panel.orientation,
                            ConfigItem.Offset:                Offset(
                                    first_offset.value - offset.value,
                                    f'{first_offset.expression} - {offset.expression}',
                                    first_offset.unitType
                            )
                        })
                        kerf_config[axes][ConfigItem.FingerCuts].append(
                                (kerf_sketch, face, finger_data)
                        )

            with PanelFingerSketch(
                    extrusion=extrusion,
                    selector=face_selectors[selected_face],
                    name=f'{panel.name} {names}',
                    start=offset,
                    end=(finger_width, panel.thickness.value),
                    transform=panel.profile_transform,
                    orientation=panel.orientation
            ) as sketch:
                for face in sorted_faces:
                    cuts_config[axes].update({
                        ConfigItem.FingerAxis:            axes,
                        ConfigItem.FingerPatternDistance: distance,
                        ConfigItem.FingerCount:           count,
                        ConfigItem.Orientation:           panel.orientation,
                        ConfigItem.Offset:                Offset(
                                first_offset.value - offset.value,
                                f'{first_offset.expression} - {offset.expression}',
                                first_offset.unitType
                        )
                    })
                    cuts_config[axes][ConfigItem.FingerCuts].append(
                            (sketch, face, finger_data)
                    )

        return cuts_config, kerf_config

    @staticmethod
    def _render_finger_cut(sketch, face, kerf):
        face_name = face[ConfigItem.Face]
        logger.debug(f'Rendering finger cuts on {face_name}:\n{pformat(face)} with {kerf}')
        set_offset = face[ConfigItem.Offset]
        real_offset = set_offset.value - face[ConfigItem.FingerDepth].value
        logger.debug(f'Real offset: {real_offset}')

        kerf_selector = {
            True:  Offset(
                    real_offset + kerf.value,
                    f'({face[ConfigItem.Offset].expression} - {face[ConfigItem.FingerDepth].expression} + {kerf.expression})',
                    set_offset.unitType
            ),
            False: Offset(
                    real_offset,
                    f'({face[ConfigItem.Offset].expression} - {face[ConfigItem.FingerDepth].expression})',
                    set_offset.unitType
            )
        }
        kerf_offset = kerf_selector[bool(kerf.value) and bool(real_offset)]

        feature = sketch.cut(kerf_offset.value, face[ConfigItem.FingerDepth].value)
        return feature

    @staticmethod
    def _configure_fingers(panel):
        finger_config = defaultdict(lambda: defaultdict(lambda: defaultdict(
                lambda: defaultdict(lambda: {
                    ConfigItem.Fingers: { },
                    ConfigItem.Faces:   []
                }))))

        finger_info = (
            (axis_id, profile, axes, finger_type, finger_data)
            for axis_id, axis_data in panel.faces.items()
            for profile, profile_data in axis_data.items()
            for axes, axes_data in profile_data.items()
            for finger_type, finger_data in axes_data.items()
        )

        thickness_values = (
            (axis, profile, axes, finger_type, finger)
            for axis, profile, axes, finger_type, finger_data in finger_info
            for fingers in finger_data.values()
            for finger in fingers
        )

        for axis, profile, axes, finger_type, finger in thickness_values:
            finger_config[axis][profile][axes][finger_type][ConfigItem.Fingers] = finger[ConfigItem.Fingers]
            finger_config[axis][profile][axes][finger_type][ConfigItem.Faces].append(finger[ConfigItem.Face])

        return finger_config


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
