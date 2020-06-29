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
from .entities import Parameter
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
    parameters = { }
    enabled = { }

    orientation = 0
    root = None
    preview = False

    def clear(self):
        self.axes.clear()
        self.names.clear()
        self.thickness_groups.clear()
        self.enabled.clear()
        self.preview = False
        self.root = None
        self.orientation = 0
        self.parameters.clear()


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

    def execute(self, *, orientation, component):
        self._repository.orientation = orientation
        self._repository.root = component

        self._processes.create_process(DefinePanels)
        self._processes.create_process(RenderPanels)
        self._processes.create_process(SaveParameters)

        self._processes._config = self._config
        self._processes._repository = self._repository
        self._processes.process()
        self.clear()

    def preview(self, *, orientation, component):
        self._repository.orientation = orientation
        self._repository.root = component
        self._repository.preview = True

        self._processes.create_process(DefinePanels)
        self._processes.create_process(RenderPanels)

        self._processes._config = self._config
        self._processes._repository = self._repository
        self._processes.process()
        self.clear()

    def clear(self):
        self._repository.clear()
        self._processes.clear()


class DefinePanels(ProcessManager):

    def _post_init_(self):
        self.create_process(ConfigurePanels)
        self.create_process(DefineEnabledPanels)


class ConfigurePanels(ProcessManager):

    def process(self):
        orientation = self._repository.orientation
        panel_instances = { }

        self._configure_parameters()
        self._configure_basic_panels(orientation)
        self._organize_profile_groups(panel_instances)
        self._configure_panels_and_faces(panel_instances)
        self._separate_thickness_groups(panel_instances)

    def _configure_parameters(self):
        logger.debug(f'Config Parameters: {pformat(self._config.parameters.items())}')
        self._repository.parameters = {
            key: {
                ConfigItem.Name: parameter[ConfigItem.Name],
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

        logger.debug(f'Parameters: {pformat(self._repository.parameters)}')

    def _configure_basic_panels(self, orientation):
        for panel, data in self._config.panels.items():
            axis = data[ConfigItem.Axis]
            name = data[ConfigItem.Name].value

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
            self._repository.names[panel.enabled][panel.axis.value][panel.profile].append(panel.name)
            self._repository.axes[panel.enabled][panel.axis.value][panel.profile] = {
                ConfigItem.ProfileTransform: panel.transform,
                ConfigItem.PlaneSelector:    panel.plane_selector,
                ConfigItem.Profile:          panel.profile
            }
            panel_instances[panel.name] = {
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
            True: f'({actual_finger_width.expression} + ({kerf.expression} * 2))',
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
            True: f'({kerf.expression})',
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

    def _separate_thickness_groups(self, panel_instances):
        for panel in (panel[ConfigItem.Panel] for panel in panel_instances.values()):
            enabled, axis, profile = panel.enabled, panel.axis.value, panel.profile
            self._repository.thickness_groups[enabled][axis][profile][panel.thickness.expression].append(panel)

    def _dimension(self, data, key, wrapper):
        return wrapper(*self._control_parameter(data[key]))

    def _control_parameter(self, key):
        control = self._config.controls[key]
        return control.value, self._config.parameters[key][ConfigItem.Name], control.unitType

    def _offset(self, data, key, wrapper, orientation, kerf):
        offset = self._dimension(data[key], orientation, wrapper)
        return wrapper(offset.value + kerf.value, f'{offset.expression} + {kerf.expression}')

    def _profile(self, data, key, kerf):
        length, width = data[key]

        profile_length = Length(*self._control_parameter(length))
        profile_width = Width(*self._control_parameter(width))

        return PanelProfile(
                Length(
                        profile_length.value + kerf.value,
                        ' + '.join([val.expression for val in filter(lambda s: s.value, [profile_length, kerf])]),
                        profile_length.units
                ),
                Width(
                        profile_width.value + kerf.value,
                        ' + '.join([val.expression for val in filter(lambda s: s.value, [profile_width, kerf])]),
                        profile_width.units
                )
        )

    def _thickness(self, data, override, key):
        logger.debug(f'Override value for {key} is {override.value}')
        selector = {
            True:  lambda d, k: self._dimension(d, k, Thickness),
            False: lambda d, k: Thickness(*self._control_parameter(Inputs.Thickness))
        }
        return selector[override.value](data, key)


class DefineEnabledPanels(Process):

    def process(self):
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

    def process(self):
        root = self._repository.root

        for axis_key, axis in self._repository.enabled.items():
            for profile in axis.items():
                self._render_profiles(root, profile)

    def _render_profiles(self, root, profile):
        app = adsk.core.Application.get()
        timeline = app.activeProduct.timeline

        start = timeline.markerPosition

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
                cut.name = f'{sketch._name} Kerf Extrusion'
                cuts.append(
                        cut
                )

            axes = cut_config[ConfigItem.FingerAxis]
            distance = cut_config[ConfigItem.FingerPatternDistance]
            count = cut_config[ConfigItem.FingerCount]
            orientation = cut_config[ConfigItem.Orientation]

            replicator = FingerCutsPattern(root, orientation)
            replicator.copy(axes=axes, cuts=cuts, distance=distance, count=count)

        for axes, cut_config in cuts_to_make.items():
            cuts = []

            for sketch, face, finger in cut_config[ConfigItem.FingerCuts]:
                cuts.append(
                        self._render_finger_cut(sketch, face, panel.kerf)
                )

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
            ConfigItem.FingerAxis: (AxisFlag.Length, AxisFlag.Width),
            ConfigItem.FingerPatternDistance: 0,
            ConfigItem.FingerCount: 0,
            ConfigItem.FingerCuts: []
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

            first_face = sorted_faces[0][ConfigItem.Face]

            if kerf.value and finger_type == FingerType.Inverse:
                length = faces_data[0][ConfigItem.Length]
                with PanelFingerSketch(
                        extrusion=extrusion,
                        selector=face_selectors[first_face],
                        name=f'{panel.name} {names} Kerf',
                        start=Offset(0, '', 'cm'),
                        end=(kerf, panel.thickness.value),
                        transform=panel.transform,
                        orientation=panel.orientation
                ) as kerf_sketch:
                    for face in sorted_faces:
                        kerf_config[axes].update({
                          ConfigItem.FingerAxis: axes,
                          ConfigItem.FingerPatternDistance: length,
                          ConfigItem.FingerCount: ActualFingerCount(2, '2'),
                          ConfigItem.Orientation: panel.orientation
                        })
                        kerf_config[axes][ConfigItem.FingerCuts].append(
                                (kerf_sketch, face, finger_data)
                        )

            with PanelFingerSketch(
                    extrusion=extrusion,
                    selector=face_selectors[first_face],
                    name=f'{panel.name} {names}',
                    start=offset,
                    end=(finger_width, panel.thickness.value),
                    transform=panel.transform,
                    orientation=panel.orientation
            ) as sketch:
                for face in sorted_faces:
                    cuts_config[axes].update({
                        ConfigItem.FingerAxis: axes,
                        ConfigItem.FingerPatternDistance: distance,
                        ConfigItem.FingerCount: count,
                        ConfigItem.Orientation: panel.orientation
                    })
                    cuts_config[axes][ConfigItem.FingerCuts].append(
                        (sketch, face, finger_data)
                    )

        return cuts_config, kerf_config

    @staticmethod
    def _render_finger_cut(sketch, face, kerf):
        set_offset = face[ConfigItem.Offset]
        real_offset = set_offset.value - face[ConfigItem.FingerDepth].value

        kerf_selector = {
            True: Offset(
                real_offset + kerf.value,
                f'({face[ConfigItem.Offset].expression} - {face[ConfigItem.FingerDepth].expression} + {kerf.expression})',
                set_offset.units
            ),
            False: Offset(
                    real_offset,
                    f'({face[ConfigItem.Offset].expression} - {face[ConfigItem.FingerDepth].expression})',
                    set_offset.units
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

        for parameter in parameters:
            self._find_or_create_parameter(parameter.name, parameter)

    def _convert_dimension_parameters(self):
        """ Convert the dimension controls to parameters, and override individual panel thickness parameters
            if required.
        """
        parameters = set()
        # Convert dimensions with parameters to straight parameters
        for key, parameter in filter(lambda p: p[1][ConfigItem.Enabled], self._repository.parameters.items()):
            logger.debug(f'Parameter: {parameter}')
            control = parameter[ConfigItem.Control]
            name = parameter[ConfigItem.Name]
            parameters.add(Parameter(name, control.value, control.unitType))
        return parameters

    def _find_or_create_parameter(self, name, parameter):
        """ Find the parameter with the given name or create that parameter and return it.
        """
        parameter_selector = {
            True: lambda e, p: self._update_parameter(e, p),
            False: lambda e, p: self._create_parameter(p)
        }

        existing_parameter = self._parameters.itemByName(name)
        return parameter_selector[bool(existing_parameter)](existing_parameter, parameter)

    def _create_parameter(self, parameter):
        """ Create the given user parameter in Fusion360.
        """
        logger.debug(f'Storing: {parameter.name} of {parameter.value} {parameter.units}')
        value = adsk.core.ValueInput.createByReal(parameter.value)
        return self._store.add(parameter.name, value, parameter.units, '')

    def _update_parameter(self, existing, parameter):
        """ Update the existing parameter with the current values specified by the user.
        """
        existing.value = parameter.value
        existing.unit = parameter.units
