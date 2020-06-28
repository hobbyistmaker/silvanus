import logging
import math
from collections import defaultdict
from pprint import pformat

import adsk.core
import adsk.fusion

from .entities import AxisDirection
from .entities import AxisFlag
from .entities import ConfigItem
from .entities import DefaultFingers
from .entities import EstimatedFingers
from .entities import ActualFingerWidth
from .entities import FingerOffset
from .entities import ActualFingerCount
from .entities import FingerPatternDistance
from .entities import FingerDepth
from .entities import FingerType
from .entities import FingerWidth
from .entities import Height
from .entities import Inputs
from .entities import Kerf
from .entities import Length
from .entities import Offset
from .entities import Override
from .entities import Panel
from .entities import PanelProfile
from .entities import Thickness
from .entities import Width
from .fusion import FaceProfile
from .fusion import FingerCutsPattern
from .fusion import PanelFingerSketch
from .fusion import PanelProfileSketch

# noinspection SpellCheckingInspection
logger = logging.getLogger('silvanus.lib.generatebox.core')


class Repository:
    axes = defaultdict(lambda: defaultdict(lambda: defaultdict(dict)))
    names = defaultdict(lambda: defaultdict(lambda: defaultdict(list)))
    panels = { }
    thickness_groups = defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: defaultdict(list))))
    enabled = { }

    def clear(self):
        self.axes.clear()
        self.names.clear()
        self.thickness_groups.clear()
        self.enabled.clear()


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

    def create_process(self, process, priority=0):
        instance = process()
        instance.priority = priority
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

    def clear(self):
        self._processes.clear()


class Core:

    def __init__(self, config):
        self._config = config
        self._repository = Repository()
        self._processes = ProcessManager()

    def execute(self, *args, **kwargs):
        self._processes.create_process(DefinePanels)
        self._processes.create_process(RenderPanels)

        self._processes._config = self._config
        self._processes._repository = self._repository
        self._processes.process(*args, **kwargs)
        self.clear()

    def preview(self, *args, **kwargs):
        self._processes.create_process(DefinePanels)

        self._processes._config = self._config
        self._processes._repository = self._repository
        self._processes.process(*args, **kwargs)
        self.clear()

    def clear(self):
        self._repository.clear()
        self._processes.clear()


class DefinePanels(ProcessManager):

    def _post_init_(self):
        self.create_process(ConfigurePanels)
        self.create_process(DefineEnabledPanels)


class ConfigurePanels(ProcessManager):

    def process(self, orientation, *_):
        panel_instances = { }

        self._configure_basic_panels(orientation)
        self._organize_profile_groups(panel_instances)
        self._configure_panels_and_faces(panel_instances)
        self._separate_thickness_groups(panel_instances)

    def _configure_basic_panels(self, orientation):
        for panel, data in self._config.panels.items():
            axis = data[ConfigItem.Axis]
            name = data[ConfigItem.Name]

            enabled = self._config.controls[data[ConfigItem.Enabled]].value

            finger_width = self._dimension(data, ConfigItem.FingerWidth, FingerWidth)
            height = self._dimension(data, ConfigItem.Height, Height)
            kerf = self._dimension(data, ConfigItem.Kerf, Kerf)
            length = self._dimension(data, ConfigItem.Length, Length)
            override = Override(self._config.controls[data[ConfigItem.Override]].value)
            thickness = self._thickness(data, override, ConfigItem.Thickness)
            offset = Offset(*self._control_parameter(data[ConfigItem.Offset][orientation]))

            profile = self._profile(data, ConfigItem.Profile, kerf)
            width = self._dimension(data, ConfigItem.Width, Width)

            planes = self._config.planes[axis.value]
            profile_transform = planes[orientation][ConfigItem.ProfileTransform]
            plane_selector = planes[orientation][ConfigItem.PlaneSelector]
            direction = AxisDirection(*planes[orientation][ConfigItem.ExtentDirection])

            face_selectors = data[ConfigItem.FaceSelectors][orientation]

            self._repository.panels[panel] = Panel(
                    axis,
                    name,
                    orientation,
                    enabled,
                    finger_width,
                    height,
                    kerf,
                    length,
                    offset,
                    override,
                    profile,
                    thickness,
                    width,
                    profile_transform,
                    plane_selector,
                    direction,
                    face_selectors
            )

    def _organize_profile_groups(self, panel_instances):
        for panel_id, panel in self._repository.panels.items():
            self._repository.names[panel.enabled][panel.axis.value][panel.profile].append(panel.name.name)
            self._repository.axes[panel.enabled][panel.axis.value][panel.profile] = {
                ConfigItem.ProfileTransform: panel.transform,
                ConfigItem.PlaneSelector:    panel.plane_selector,
                ConfigItem.Profile:          panel.profile
            }
            panel_instances[panel.name.name] = {
                ConfigItem.Panel:     panel,
                ConfigItem.PanelData: self._config.panels[panel_id]
            }

    def _configure_panels_and_faces(self, panel_instances):

        for name, panel_info in panel_instances.items():
            panel = panel_info[ConfigItem.Panel]
            data = panel_info[ConfigItem.PanelData]
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

            length = parameters[face_data[ConfigItem.Length]]
            width = parameters[face_data[ConfigItem.Width]]
            profile = (length, width)
            finger_type = face_data[ConfigItem.FingerType]

            jointed_panel = self._repository.panels[face_data[ConfigItem.Joint]]
            panel_offset = jointed_panel.offset
            depth = jointed_panel.thickness

            kerf = panel.kerf
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
                ConfigItem.Fingers: finger_selector[finger_type](length, panel.finger_width),
                ConfigItem.Face:    face_config
            }

            faces_repository[enabled][axis][profile][axes][finger_type][depth.expression].append(finger_data)

        return faces_repository[True]

    @staticmethod
    def _configure_normal_fingers(length, finger_width):
        default_fingers = DefaultFingers(
            math.ceil(length.value / finger_width.value),
            f'ceil({length.expression} / {finger_width.expression})'
        )
        estimated_fingers = EstimatedFingers(
            max(3, (math.floor(default_fingers.value / 2) * 2) - 1),
            f'max(3; (floor({default_fingers.expression} / 2) * 2) - 1)'
        )
        actual_finger_width = ActualFingerWidth(
            length.value / estimated_fingers.value,
            f'{length.expression} / {estimated_fingers.expression}'
        )
        finger_offset = FingerOffset(
            actual_finger_width.value,
            f'{actual_finger_width.expression}'
        )
        actual_number_fingers = ActualFingerCount(
            math.floor(estimated_fingers.value / 2),
            f'floor({estimated_fingers.expression} / 2)'
        )
        distance = FingerPatternDistance(
            (estimated_fingers.value - 3) * actual_finger_width.value,
            f'({estimated_fingers.expression} - 3) * {actual_finger_width.expression}'
        )

        return {
            ConfigItem.FingerType:            FingerType.Normal,
            ConfigItem.FingerOffset:          finger_offset,
            ConfigItem.FingerCount:           actual_number_fingers,
            ConfigItem.FingerWidth:           actual_finger_width,
            ConfigItem.FingerPatternDistance: distance,
        }

    @staticmethod
    def _configure_inverse_fingers(length, finger_width):
        default_fingers = DefaultFingers(
                math.ceil(length.value / finger_width.value),
                f'ceil({length.expression} / {finger_width.expression})'
        )
        estimated_fingers = EstimatedFingers(
                max(3, (math.floor(default_fingers.value / 2) * 2) - 1),
                f'max(3; (floor({default_fingers.expression} / 2) * 2) - 1)'
        )
        actual_finger_width = ActualFingerWidth(
                length.value / estimated_fingers.value,
                f'{length.expression} / {estimated_fingers.expression}'
        )
        finger_offset = FingerOffset(
                0,
                ''
        )
        actual_number_fingers = ActualFingerCount(
                math.ceil(estimated_fingers.value / 2),
                f'ceil({estimated_fingers.expression} / 2)'
        )
        distance = FingerPatternDistance(
                (estimated_fingers.value - 1) * actual_finger_width.value,
                f'({estimated_fingers.expression} - 1) * {actual_finger_width.expression}'
        )

        return {
            ConfigItem.FingerType:            FingerType.Inverse,
            ConfigItem.FingerOffset:          finger_offset,
            ConfigItem.FingerCount:           actual_number_fingers,
            ConfigItem.FingerWidth:           actual_finger_width,
            ConfigItem.FingerPatternDistance: distance,
        }

    def _separate_thickness_groups(self, panel_instances):
        for panel in (panel[ConfigItem.Panel] for panel in panel_instances.values()):
            enabled, axis, profile = panel.enabled, panel.axis.value, panel.profile
            self._repository.thickness_groups[enabled][axis][profile][panel.thickness.expression].append(panel)

    def _dimension(self, data, key, wrapper):
        return wrapper(*self._control_parameter(data[key]))

    def _control_parameter(self, key):
        return self._config.controls[key].value, self._config.parameters[key]

    def _offset(self, data, key, wrapper, orientation):
        return wrapper(*self._control_parameter(data[key][orientation]))

    def _profile(self, data, key, kerf):
        length, width = data[key]

        profile_length = Length(*self._control_parameter(length))
        profile_width = Width(*self._control_parameter(width))

        return PanelProfile(
                Length(
                        profile_length.value + kerf.value,
                        ' + '.join([val.expression for val in filter(lambda s: s.value, [profile_length, kerf])])
                ),
                Width(
                        profile_width.value + kerf.value,
                        ' + '.join([val.expression for val in filter(lambda s: s.value, [profile_width, kerf])])
                )
        )

    def _thickness(self, data, override, key):
        selector = {
            True:  lambda d, k: self._dimension(d, k, Thickness),
            False: lambda d, k: Thickness(*self._control_parameter(Inputs.Thickness))
        }
        return selector[override.value](data, key)


class DefineEnabledPanels(Process):

    def process(self, *_, **__):
        enabled_panels = self._repository.thickness_groups[True]

        self._repository.enabled = {
            axis: {
                profile_key: {
                    ConfigItem.ProfileName:      '-'.join(self.get_panel_names(axis, profile_key)),
                    ConfigItem.ThicknessGroups:  groups,
                    ConfigItem.ProfileTransform: self.get_profile_transform_function(axis, profile_key),
                    ConfigItem.PlaneSelector:    self.get_plane_selector(axis, profile_key),
                    ConfigItem.Profile:          self.get_profile_dimensions(axis, profile_key),
                    ConfigItem.Faces:            self.get_faces(groups),
                    ConfigItem.FaceSelectors:    self.get_face_selectors(groups)
                }
            }
            for axis, profile_data in enabled_panels.items()
            for profile_key, groups in profile_data.items()
        }

    def get_panel_names(self, axis, profile_key):
        return self._repository.names[True][axis][profile_key]

    def get_profile_transform_function(self, axis, profile_key):
        return self._repository.axes[True][axis][profile_key][ConfigItem.ProfileTransform]

    def get_plane_selector(self, axis, profile_key):
        return self._repository.axes[True][axis][profile_key][ConfigItem.PlaneSelector]

    def get_profile_dimensions(self, axis, profile_key):
        return self._repository.axes[True][axis][profile_key][ConfigItem.Profile]

    @staticmethod
    def get_faces(groups):
        for panel in (
                panel_data
                for group in groups.values()
                for panel_data in group
        ):
            return panel.faces

    @staticmethod
    def get_face_selectors(groups):
        for panel in (
                panel_data
                for group in groups.values()
                for panel_data in group
        ):
            return panel.face_selectors


class RenderPanels(ProcessManager):

    def process(self, orientation, root):
        for axis_key, axis in self._repository.enabled.items():
            for profile in axis.items():
                self._render_profiles(root, profile)

    def _render_profiles(self, root, profile):
        profile_key, profile_data = profile

        name = profile_data[ConfigItem.ProfileName]
        selector = profile_data[ConfigItem.PlaneSelector]
        transform = profile_data[ConfigItem.ProfileTransform]
        dimensions = profile_data[ConfigItem.Profile]

        with PanelProfileSketch(
                plane_selector=selector(root), transform=transform,
                end=(dimensions.length.value, dimensions.width.value), name=name
        ) as profile_sketch:
            for group, panels in profile_data[ConfigItem.ThicknessGroups].items():
                self._extrude_profiles(root, profile_sketch, group, panels)

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
        extrusion.name = f'{panel.name.name} Panel Extrusion'
        body = extrusion.bodies.item(0)
        body.name = f'{panel.name.name} Panel Body'

        cuts_to_make = self._render_finger_profiles(panel, extrusion)
        for axes, cut_config in cuts_to_make.items():
            cuts = []

            for sketch, face, finger in cut_config[ConfigItem.FingerCuts]:
                cuts.append(
                        self._render_finger_cut(sketch, face, finger)
                )

            axes = cut_config[ConfigItem.FingerAxis]
            distance = cut_config[ConfigItem.FingerPatternDistance]
            count = cut_config[ConfigItem.FingerCount]
            orientation = cut_config[ConfigItem.Orientation]

            replicator = FingerCutsPattern(root, orientation)
            replicator.copy(axes=axes, cuts=cuts, distance=distance, count=count)

        return extrusion

    @staticmethod
    def _render_copy_extrusion(_, reference, panel):
        with FaceProfile(reference, name='') as profile:
            extrusion = profile.extrude(panel.offset, panel.thickness, panel.direction)
            extrusion.name = f'{panel.name.name} Panel Extrusion'
            body = extrusion.bodies.item(0)
            body.name = f'{panel.name.name} Panel Body'

        return reference

    def _render_finger_profiles(self, panel, extrusion):
        face_selectors = panel.face_selectors

        finger_config = self._configure_fingers(panel)

        cuts_config = defaultdict(lambda: {
            ConfigItem.FingerAxis:            (AxisFlag.Length, AxisFlag.Width),
            ConfigItem.FingerPatternDistance: 0,
            ConfigItem.FingerCount:           0,
            ConfigItem.FingerCuts:            []
        })

        for axis, axes_data in finger_config.items():
            for profile, profile_data in axes_data.items():
                for axes, finger_type_data in profile_data.items():
                    for finger_type, finger_type_config in finger_type_data.items():
                        faces_data = finger_type_config[ConfigItem.Faces]
                        finger_data = finger_type_config[ConfigItem.Fingers]

                        offset = finger_data[ConfigItem.FingerOffset]
                        finger_width = finger_data[ConfigItem.FingerWidth]

                        distance = finger_data[ConfigItem.FingerPatternDistance]
                        count = finger_data[ConfigItem.FingerCount]

                        sorted_faces = sorted(faces_data, key=lambda f: f[ConfigItem.Offset].value)
                        names = '-'.join([item[ConfigItem.Face].name for item in sorted_faces])

                        first_face = sorted_faces[0][ConfigItem.Face]

                        with PanelFingerSketch(
                                extrusion=extrusion,
                                selector=face_selectors[first_face],
                                name=f'{panel.name.name} {names}',
                                start=offset,
                                end=(finger_width, panel.thickness.value),
                                transform=panel.transform,
                                orientation=panel.orientation
                        ) as sketch:
                            for face in sorted_faces:
                                cuts_config[axes].update({
                                    ConfigItem.FingerAxis:            axes,
                                    ConfigItem.FingerPatternDistance: distance,
                                    ConfigItem.FingerCount:           count,
                                    ConfigItem.Orientation:           panel.orientation
                                })
                                cuts_config[axes][ConfigItem.FingerCuts].append(
                                        (sketch, face, finger_data)
                                )

        return cuts_config

    @staticmethod
    def _render_finger_cut(sketch, face, finger):
        real_offset = face[ConfigItem.Offset].value - face[ConfigItem.FingerDepth].value
        feature = sketch.cut(real_offset, face[ConfigItem.FingerDepth].value)
        return feature

    @staticmethod
    def _configure_fingers(panel):
        finger_config = defaultdict(lambda: defaultdict(lambda: defaultdict(
                lambda: defaultdict(lambda: {
                    ConfigItem.Fingers: { },
                    ConfigItem.Faces:   []
                }))))

        finger_info = [
            (axis_id, profile, axes, finger_type, finger_data)
            for axis_id, axis_data in panel.faces.items()
            for profile, profile_data in axis_data.items()
            for axes, axes_data in profile_data.items()
            for finger_type, finger_data in axes_data.items()
        ]

        thickness_values = [
            (axis, profile, axes, finger_type, fingers)
            for axis, profile, axes, finger_type, finger_data in finger_info
            for fingers in finger_data.values()
        ]

        for axis, profile, axes, finger_type, fingers in thickness_values:
            for finger in fingers:
                depth = finger[ConfigItem.Face][ConfigItem.FingerDepth].expression
                finger_config[axis][profile][axes][finger_type][ConfigItem.Fingers] = finger[ConfigItem.Fingers]
                finger_config[axis][profile][axes][finger_type][ConfigItem.Faces].append(finger[ConfigItem.Face])
        return finger_config
