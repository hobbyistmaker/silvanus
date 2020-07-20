import logging
import math
from collections import defaultdict
from collections import namedtuple

from .ecs import Process
from .entities import ActualFingerCount
from .entities import ActualFingerWidth
from .entities import AxisFlag
from .entities import ConfigItem
from .entities import DefaultFingers
from .entities import DividerCount
from .entities import DividerCountInput
from .entities import EnableInput
from .entities import Enabled
from .entities import EstimatedFingers
from .entities import ExtrusionDistance
from .entities import FingerOffset
from .entities import FingerOrientation
from .entities import FingerPatternDistance
from .entities import FingerPatternType
from .entities import FingerType
from .entities import FingerWidth
from .entities import FingerWidthInput
from .entities import Height
from .entities import HeightInput
from .entities import HeightOrientation
from .entities import Inputs
from .entities import InsidePanel
from .entities import InverseFingerPattern
from .entities import JointItem
from .entities import JointName
from .entities import JointPanelOffset
from .entities import JointPatternDistance
from .entities import JointProfile
from .entities import JointThickness
from .entities import Kerf
from .entities import KerfInput
from .entities import KerfJoint
from .entities import Length
from .entities import LengthInput
from .entities import LengthOrientation
from .entities import MaxOffset
from .entities import MaxOffsetInput
from .entities import NormalFingerPattern
from .entities import PanelEndReferencePoint
from .entities import PanelJoint
from .entities import PanelName
from .entities import PanelOffset
from .entities import PanelOrientation
from .entities import PanelProfile
from .entities import PanelStartReferencePoint
from .entities import ParentPanel
from .entities import ReferencePointInput
from .entities import Renderable
from .entities import Thickness
from .entities import ThicknessInput
from .entities import Width
from .entities import WidthInput
from .entities import WidthOrientation

# noinspection SpellCheckingInspection
logger = logging.getLogger('silvanus.lib.generatebox.core')


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
        self._del_unused_kerf()
        self._add_finger_width_inputs()
        self._add_max_offset_inputs()
        self._add_divider_count_inputs()

        self._add_dividers()
        self._add_extrusion_distance()
        self._add_reference_points()

        self._add_orientations()
        self._add_height_orientation()
        self._add_length_orientation()
        self._add_width_orientation()

        self._add_height_panel_start_points()
        self._add_length_panel_start_points()
        self._add_width_panel_start_points()

        self._add_kerf_inputs()
        self._kerf_adjust_inside_length_panels()
        self._kerf_adjust_profiles()

        self._define_finger_joints()
        self._add_kerf_to_joints()
        self._add_finger_pattern_component()
        self._add_normal_finger_parameters()
        self._add_inverse_finger_parameters()

        self._kerf_adjust_fingers()
        self._create_kerf_joints()
        self._add_joint_profile_groups()

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

    def _read_float_control(self, source, dest):
        """ Read the value from a Fusion360 input control """
        control = self._config.controls[source.control]
        return dest(control.value, control.name, control.unitType)

    def _read_int_control(self, source, dest):
        control = self._config.controls[source.control]
        return dest(control.value, control.name)

    def _enable_panels(self):
        """ For each entity with an EnableInput control, read the value and create an Enable component if the
            EnableInput value is true.
        """

        def read_enabled(entity):
            return entity.EnableInput.control.value

        self._repository.with_components(EnableInput).with_true(read_enabled).for_each(
                lambda c: c.add_component(Enabled(True))
        )

    def _disable_panels(self):
        """ For each entity with an EnableInput control and an Enable component, read the value and delete the
            corresponding Enable component, if the EnableInput value is false.
        """

        def read_enabled(entity):
            return not entity.EnableInput.control.value

        self._repository.with_components(EnableInput, Enabled).with_true(read_enabled).for_each(
                lambda c: c.remove_component(Enabled)
        )

    def _add_thickness_inputs(self):
        """ For each entity with a ThicknessInput control, read the value and create or replace a Thickness
            component.
        """
        self._repository.with_components(ThicknessInput).for_each(
                lambda c: c.add_component(self._read_float_control(c.ThicknessInput, Thickness))
        )

    def _add_length_inputs(self):
        """ For each entity with a LengthInput control, read the value and create or replace a Length component.
        """
        self._repository.with_components(LengthInput).for_each(
                lambda c: c.add_component(self._read_float_control(c.LengthInput, Length))
        )

    def _add_width_inputs(self):
        """ For each entity with a WidthInput control, read the value and create or replace a Width component.
        """
        self._repository.with_components(WidthInput).for_each(
                lambda c: c.add_component(self._read_float_control(c.WidthInput, Width))
        )

    def _add_height_inputs(self):
        """ For each entity with a HeightInput control, read the value and create or replace the Height component.
        """
        self._repository.with_components(HeightInput).for_each(
                lambda c: c.add_component(self._read_float_control(c.HeightInput, Height))
        )

    def _add_divider_count_inputs(self):
        def read_input(entity):
            control = self._config.controls[entity.DividerCountInput.control]
            logger.debug(f'Divider Count: {control.value}')
            return bool(control.value)

        def add_divider_count(entity):
            logger.debug('Adding Divider Count Entity')
            entity.add_component(self._read_int_control(entity.DividerCountInput, DividerCount))

        self._repository.with_components(DividerCountInput).with_true(read_input).for_each(add_divider_count)

    def _add_extrusion_distance(self):
        """ For each entity with a Thickness component, create a corresponding ExtrusionDistance component. This
            will be used in any rendering processes, and may end up being a modified version of the Thickness
            values, depending on processes that run after this.
        """
        self._repository.with_components(Thickness).for_each(
                lambda c: c.add_component(
                        ExtrusionDistance(c.Thickness.value, c.Thickness.expression, c.Thickness.unitType)
                )
        )

    def _add_kerf_inputs(self):
        """ For each entity with a KerfInput component, read the value and create or replace the Kerf component if
            the kerf value is greater than 0.
        """

        def read_kerf(entity):
            control = self._config.controls[entity.KerfInput.control]
            return bool(control.value)

        self._repository.with_components(KerfInput).with_true(read_kerf).for_each(
                lambda c: c.add_component(self._read_float_control(c.KerfInput, Kerf))
        )

    def _del_unused_kerf(self):
        """ For each entity with a KerfInput component, read the value and create or replace the Kerf component if
            the kerf value is greater than 0.
        """

        def read_kerf(entity):
            control = self._config.controls[entity.KerfInput.control]
            return not bool(control.value)

        self._repository.with_components(KerfInput, Kerf).with_true(read_kerf).for_each(
                lambda c: c.remove_component(Kerf)
        )

    def _add_finger_width_inputs(self):
        """ For each entity with a FingerWidth component, read the value and create or replace a FingerWidth
            component.
        """
        self._repository.with_components(FingerWidthInput).for_each(
                lambda c: c.add_component(self._read_float_control(c.FingerWidthInput, FingerWidth))
        )

    def _add_max_offset_inputs(self):
        """ For each entity with a MaxOffsetInput component, read the value and create or replace a MaxOffset
            component.
        """

        def read_max_offset(entities):
            for entity in entities:
                control = self._config.controls[entity.MaxOffsetInput.control]
                entity.add_component(MaxOffset(control.value, control.name, control.unitType))

        self._repository.with_components(MaxOffsetInput).with_all(read_max_offset)

    def _add_dividers(self):
        divider_group = self._repository.with_components(
                DividerCount, Length, Width, Height, Thickness, FingerWidth, PanelOrientation, MaxOffset, KerfInput
        ).instances

        logger.debug(f'{len(divider_group)} divider groups.')
        for dividers in divider_group:
            divider_count = dividers.DividerCount.value
            pocket_count = divider_count + 1
            total_panels = divider_count + 2
            pocket_offset = (dividers.MaxOffset.value - dividers.Thickness.value * total_panels) / pocket_count
            thickness = dividers.Thickness.value
            for panel_num in range(1, pocket_count):
                panel_entity = self._repository.create(
                        Enabled(True),
                        InsidePanel(),
                        PanelName(f'Length Divider {panel_num}'),
                        dividers.Thickness,
                        dividers.Length,
                        dividers.Width,
                        dividers.Height,
                        dividers.FingerWidth,
                        dividers.PanelOrientation,
                        dividers.MaxOffset,
                        dividers.KerfInput,
                        PanelEndReferencePoint(
                                Length((pocket_offset * panel_num) + thickness*(1+panel_num), f'{(pocket_offset * panel_num) + thickness*(1+panel_num)}', dividers.Length.unitType),
                                Width(dividers.Width.value, dividers.Width.expression, dividers.Width.unitType),
                                Height(dividers.Height.value, dividers.Height.expression, dividers.Height.unitType)
                        )
                )

                logger.debug(f'Adding inside panel at {(pocket_offset * panel_num) + thickness} for {panel_entity.id}.')

                for finger_type, axes in { FingerType.Inverse: [AxisFlag.Height, AxisFlag.Width] }.items():
                    for axis in axes:
                        self._repository.create(
                                Enabled(True),
                                FingerOrientation(axis),
                                FingerPatternType(finger_type),
                                ParentPanel(panel_entity.id)
                        )

    def _add_orientations(self):
        """ For each entity with a PanelOrientation component, read the value and create or replace a
            corresponding LengthOrientation, WidthOrientation or HeightOrientation component.
        """
        selector = {
            AxisFlag.Height: HeightOrientation,
            AxisFlag.Width:  WidthOrientation,
            AxisFlag.Length: LengthOrientation
        }
        self._repository.with_components(PanelOrientation).for_each(
                lambda c: c.add_component(selector[c.PanelOrientation.axis]())
        )

    def _add_height_orientation(self):
        """ For entities with a Height Orientation component, create the PanelProfile component with Length(length),
            and Width(width).
        """
        self._repository.with_components(Enabled, HeightOrientation, Length, Width).for_each(
                lambda c: c.add_components(PanelProfile(c.Length, c.Width))
        )

    def _add_length_orientation(self):
        """ For entities with a LengthOrientation component, create the PanelProfile component with Length(width)
            and Width(height).
        """
        self._repository.with_components(Enabled, LengthOrientation, Width, Height).for_each(
                lambda c: c.add_components(PanelProfile(c.Width, c.Height))
        )

    def _add_width_orientation(self):
        """ For entities with a WidthOrientation component, create the PanelProfile component with Length(length)
            and Height(height).
        """
        self._repository.with_components(Enabled, WidthOrientation, Length, Height).for_each(
                lambda c: c.add_components(PanelProfile(c.Length, c.Height))
        )

    def _add_reference_points(self):
        """ For entities with a ReferencePointInput component, create a PanelEndReferencePoint component
            that will populate the length, width, height of the component from the associated Inputs in the dialog.
        """

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

    def _add_height_panel_start_points(self):
        """ For entities with HeightOrientation, PanelEndReferencePoint and Thickness components, create a
            PanelStartReferencePoint component with (l, w, h) == (0, 0, 'height - thickness') from the
            PanelEndReferencePoint; i.e. set the start point to be closest to the origin of the model.
        """

        def add_start_point(entity):
            end = entity.PanelEndReferencePoint
            thickness = entity.ExtrusionDistance

            expression = {
                False: '0',
                True:  f'{end.height.expression} - {thickness.expression}'
            }
            height_value = end.height.value - thickness.value
            start_height = Height(
                    height_value,
                    expression[bool(height_value)],
                    end.height.unitType
            )
            entity.add_components(
                    PanelStartReferencePoint(
                            Length(0, '', end.length.unitType),
                            Width(0, '', end.width.unitType),
                            height_value
                    ),
                    PanelOffset(start_height.value, start_height.expression, start_height.unitType),
                    JointPanelOffset(start_height.value, start_height.expression, start_height.unitType)
            )

        self._repository.with_components(HeightOrientation, PanelEndReferencePoint, ExtrusionDistance).for_each(add_start_point)

    def _add_length_panel_start_points(self):
        """ For entities with LengthOrientation, PanelEndReferencePoint and Thickness components, create a
            PanelStartReferencePoint component with (l, w, h) == ('length - thickness', 0, 0) from the
            PanelEndReferencePoint; i.e. set the start point to be closest to the origin of the model.
        """

        def add_start_point(entity):
            end = entity.PanelEndReferencePoint
            thickness = entity.ExtrusionDistance

            expression = {
                False: '0',
                True:  f'{end.length.expression} - {thickness.expression}'
            }

            length_value = end.length.value - thickness.value
            start_length = Length(
                    length_value,
                    expression[bool(length_value)],
                    end.length.unitType
            )

            entity.add_components(
                    PanelStartReferencePoint(
                            start_length,
                            Width(0, '', end.width.unitType),
                            Height(0, '', end.height.unitType)
                    ),
                    PanelOffset(start_length.value, start_length.expression, start_length.unitType),
                    JointPanelOffset(start_length.value, start_length.expression, start_length.unitType)
            )

        self._repository.with_components(LengthOrientation, PanelEndReferencePoint, ExtrusionDistance).for_each(add_start_point)

    def _add_width_panel_start_points(self):
        """ For entities with WidthOrientation, PanelEndReferencePoint and Thickness components, create a
            PanelStartReferencePoint component with (l, w, h) == (0, 'width - thickness', 0) from the
            PanelEndReferencePoint; i.e. set the start point to be closest to the origin of the model.
        """

        def add_start_point(entity):
            end = entity.PanelEndReferencePoint
            thickness = entity.ExtrusionDistance

            expression = {
                False: '0',
                True:  f'{end.width.expression} - {thickness.expression}'
            }

            width_value = end.width.value - thickness.value
            start_width = Width(
                    width_value,
                    expression[bool(width_value)],
                    end.width.unitType
            )

            entity.add_components(
                    PanelStartReferencePoint(
                            Length(0, '', end.length.unitType),
                            start_width,
                            Height(0, '', end.width.unitType)
                    ),
                    PanelOffset(start_width.value, start_width.expression, start_width.unitType),
                    JointPanelOffset(start_width.value, start_width.expression, start_width.unitType)
            )

        self._repository.with_components(WidthOrientation, PanelEndReferencePoint, ExtrusionDistance).for_each(add_start_point)

    def _kerf_adjust_inside_length_panels(self):
        panels = self._repository.with_components(
                InsidePanel, LengthOrientation, Thickness, Kerf, JointPanelOffset
        ).instances
        for panel in panels:
            offset = panel.JointPanelOffset
            thickness = panel.Thickness
            kerf = panel.Kerf

            panel.add_components(
                    Thickness(
                            thickness.value - kerf.value,
                            f'{thickness.expression} - {kerf.expression}',
                            thickness.unitType
                    ),
                    JointPanelOffset(
                            offset.value - kerf.value/2,
                            f'{offset.expression} - {kerf.expression}/2',
                            offset.unitType
                    )
            )

    def _kerf_adjust_profiles(self):
        """ For entities with a PanelProfile component, adjust the length and width with the value of kerf, if
            kerf is set.
        """

        def add_kerf(component):
            profile, kerf = component.PanelProfile, component.Kerf
            component.add_component(PanelProfile(
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
            ))

        self._repository.with_components(PanelProfile, Kerf).for_each(add_kerf)

    def _define_finger_joints(self):
        FingerPanelJoin = namedtuple('FingerPanelJoin', [
            'id', 'FingerOrientation', 'FingerPatternType', 'PanelName',
            'PanelOrientation', 'ExtrusionDistance', 'PanelProfile',
            'PanelOffset', 'FingerWidth', 'Thickness'
        ])

        # Hash map for joining finger configurations with their parent panels
        finger_configs = defaultdict(lambda: {
            JointItem.FingerConfig: [],
            JointItem.ParentPanel:  None
        })

        # Retrieve the finger configuration: axis and joint orientation and finger pattern type
        config_rows = self._repository.with_components(
                Enabled, FingerOrientation, FingerPatternType, ParentPanel
        ).instances
        for instance in config_rows:
            finger_configs[instance.ParentPanel.id][JointItem.FingerConfig].append(instance)

        # Retrieve the parent panel information
        panel_rows = self._repository.with_components(
                Enabled, PanelName, PanelOrientation, PanelProfile, ExtrusionDistance, PanelOffset, FingerWidth,
                Thickness
        ).instances
        for instance in panel_rows:
            finger_configs[instance.id][JointItem.ParentPanel] = instance

        finger_panel_joins = (
            FingerPanelJoin(parent.id, finger.FingerOrientation, finger.FingerPatternType,
                            parent.PanelName, parent.PanelOrientation, parent.ExtrusionDistance,
                            parent.PanelProfile, parent.PanelOffset, parent.FingerWidth,
                            parent.Thickness)
            for parent, fingers in
            (
                (record_join[JointItem.ParentPanel], record_join[JointItem.FingerConfig])
                for record_join in finger_configs.values()
            )
            for finger in fingers
        )

        # Hash map for joining finger configurations with joined panels
        panels = defaultdict(lambda: {
            JointItem.ParentPanels: [],
            JointItem.JointPanels:  []
        })
        for panel in finger_panel_joins:
            panels[panel.FingerOrientation.axis][JointItem.ParentPanels].append(panel)

        joined_panels = self._repository.with_components(
                Enabled, PanelOrientation, JointPanelOffset, Length, Width, Height, PanelName, ExtrusionDistance, Thickness
        ).instances

        distance_selector = {
            (AxisFlag.Width, AxisFlag.Height):  lambda c: c.Length,
            (AxisFlag.Width, AxisFlag.Length):  lambda c: c.Height,
            (AxisFlag.Height, AxisFlag.Length): lambda c: c.Width,
            (AxisFlag.Height, AxisFlag.Width):  lambda c: c.Length,
            (AxisFlag.Length, AxisFlag.Width):  lambda c: c.Height,
            (AxisFlag.Length, AxisFlag.Height): lambda c: c.Width
        }
        for instance in joined_panels:
            joint = panels[instance.PanelOrientation.axis]
            joint[JointItem.JointPanels].append(instance)

        # Create the joint components for each panel based on the panels that they are joined with
        panel_joint_joins = (
            (parent_panel, joined_panel)
            for joined_panels, parent_panels in
            (
                (record_join[JointItem.JointPanels], record_join[JointItem.ParentPanels])
                for record_join in panels.values()
            )
            for parent_panel in parent_panels
            for joined_panel in joined_panels
        )
        for parent_panel, joined_panel in panel_joint_joins:
            total_distance = distance_selector[(parent_panel.FingerOrientation.axis,
                                                parent_panel.PanelOrientation.axis)](joined_panel)
            logger.debug(
                    f'Join {parent_panel.PanelName.value}:{parent_panel.PanelOrientation.axis}:{parent_panel.id} with profile ({(parent_panel.PanelProfile.length, parent_panel.PanelProfile.width)}) with {joined_panel.PanelName.value} along {total_distance.expression} with {parent_panel.FingerPatternType.finger_type} joint of {joined_panel.ExtrusionDistance.expression} and offset of {joined_panel.JointPanelOffset.expression}.')
            self._repository.create(
                    Renderable(),
                    PanelJoint(),
                    parent_panel.PanelName,
                    parent_panel.PanelOrientation,
                    parent_panel.PanelProfile,
                    parent_panel.FingerPatternType,
                    parent_panel.FingerOrientation,
                    parent_panel.FingerWidth,
                    parent_panel.PanelOffset,
                    parent_panel.ExtrusionDistance,
                    parent_panel.Thickness,
                    ParentPanel(parent_panel.id),
                    JointThickness(
                            joined_panel.Thickness.value,
                            joined_panel.Thickness.expression,
                            joined_panel.Thickness.unitType
                    ),
                    JointName(joined_panel.PanelName.value),
                    JointPatternDistance(
                            total_distance.value,
                            total_distance.expression,
                            total_distance.unitType
                    ),
                    joined_panel.JointPanelOffset
            )

    def _add_kerf_to_joints(self):
        parents = self._repository.with_components(Enabled, PanelName, Kerf).instances

        if not parents:
            return

        joints = self._repository.with_components(JointName, ParentPanel).instances

        left_join = defaultdict(lambda: { 'joints': [], 'parent': None })

        for joint in joints:
            left_join[joint.ParentPanel.id]['joints'].append(joint)

        for parent in parents:
            left_join[parent.id]['parent'] = parent

        for instance in left_join.values():
            parent = instance['parent']
            if not parent:
                logger.debug(f'No parent found for {instance}')
                continue
            for joint in instance['joints']:
                joint.add_component(parent.Kerf)

    def _add_finger_pattern_component(self):
        selector = {
            FingerType.Normal:  NormalFingerPattern(),
            FingerType.Inverse: InverseFingerPattern()
        }
        self._repository.with_components(FingerPatternType).for_each(
                lambda entity: entity.add_component(selector[entity.FingerPatternType.finger_type])
        )

    def _add_normal_finger_parameters(self):
        joints = self._repository.with_components(
                NormalFingerPattern, FingerWidth, JointPatternDistance
        ).instances

        logger.debug(f'{len(joints)} normal finger joints')
        for joint in joints:
            finger_width = joint.FingerWidth
            length = joint.JointPatternDistance

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
                    f'{length.expression} / {estimated_fingers.expression}',
                    finger_width.unitType
            )

            finger_offset = FingerOffset(
                    actual_finger_width.value,
                    f'{actual_finger_width.expression}',
                    finger_width.unitType
            )

            actual_number_fingers = ActualFingerCount(
                    math.floor(estimated_fingers.value / 2),
                    f'floor({estimated_fingers.expression} / 2)'
            )

            distance = FingerPatternDistance(
                    (estimated_fingers.value - 3) * actual_finger_width.value,
                    f'({estimated_fingers.expression} - 3) * ({actual_finger_width.expression}',
                    finger_width.unitType
            )

            joint.add_components(
                    default_fingers,
                    estimated_fingers,
                    actual_finger_width,
                    finger_offset,
                    actual_number_fingers,
                    distance
            )

    def _add_inverse_finger_parameters(self):
        joints = self._repository.with_components(
                InverseFingerPattern, FingerWidth, JointPatternDistance
        ).instances

        logger.debug(f'{len(joints)} inverse finger joints')
        for joint in joints:
            finger_width = joint.FingerWidth
            length = joint.JointPatternDistance

            default_fingers = DefaultFingers(
                    math.ceil(length.value / finger_width.value),
                    f'ceil({length.expression} / {finger_width.expression})'
            )
            estimated_fingers = EstimatedFingers(
                    max(3, (math.floor(default_fingers.value / 2) * 2) - 1),
                    f'max(3; (floor({default_fingers.expression} / 2) * 2) - 1)'
            )

            desired_finger_width = length.value / estimated_fingers.value

            actual_finger_width = ActualFingerWidth(
                    (length.value / estimated_fingers.value),
                    f'{length.expression} / {estimated_fingers.expression}',
                    finger_width.unitType
            )

            finger_offset = FingerOffset(0, '', finger_width.unitType)

            actual_number_fingers = ActualFingerCount(
                    math.ceil(estimated_fingers.value / 2),
                    f'ceil({estimated_fingers.expression} / 2)'
            )

            distance = FingerPatternDistance(
                    (estimated_fingers.value - 1) * desired_finger_width,
                    f'({estimated_fingers.expression} - 1)',
                    finger_width.unitType
            )

            joint.add_components(
                    default_fingers,
                    estimated_fingers,
                    actual_finger_width,
                    finger_offset,
                    actual_number_fingers,
                    distance
            )

    def _kerf_adjust_fingers(self):
        joints = self._repository.with_components(
                ActualFingerWidth, FingerOffset, Kerf, JointPanelOffset
        ).instances

        for joint in joints:
            actual_finger_width = joint.ActualFingerWidth
            finger_offset = joint.FingerOffset
            kerf = joint.Kerf

            offset_selector = {
                False: joint.JointPanelOffset,
                True:  JointPanelOffset(
                        joint.JointPanelOffset.value + kerf.value,
                        f'{joint.JointPanelOffset.expression} + {kerf.expression}',
                        joint.JointPanelOffset.unitType
                )
            }
            joint_panel_offset = offset_selector[bool(joint.JointPanelOffset.value)]

            joint.add_components(
                    ActualFingerWidth(
                            actual_finger_width.value - kerf.value,
                            f'({actual_finger_width.expression}) - ({kerf.expression})',
                            actual_finger_width.unitType
                    ),
                    FingerOffset(
                            finger_offset.value + kerf.value,
                            ' + '.join(
                                    filter(lambda f: bool(f),
                                           [f'{finger_offset.expression}', f'({kerf.expression})'])),
                            finger_offset.unitType
                    ),
                    joint_panel_offset
            )

    def _create_kerf_joints(self):
        joints = self._repository.with_components(
                PanelName, PanelOrientation, PanelProfile, FingerPatternType, FingerOrientation,
                FingerWidth, PanelOffset, ExtrusionDistance, Thickness, ParentPanel,
                JointThickness, JointName, JointPatternDistance, JointPanelOffset,
                Kerf, InverseFingerPattern
        ).instances
        logger.debug(f'{len(joints)} joints')

        for joint in joints:
            logger.debug(f'Creating kerf joint: {joint}')
            pattern_distance = joint.JointPatternDistance
            kerf = joint.Kerf

            entity = self._repository.create(
                    Renderable(),
                    KerfJoint(),
                    joint.PanelProfile,
                    joint.PanelOrientation,
                    joint.PanelName,
                    joint.ExtrusionDistance,
                    joint.PanelOffset,
                    joint.JointPatternDistance,
                    joint.JointPanelOffset,
                    joint.JointName,
                    joint.JointThickness,
                    joint.FingerOrientation,
                    FingerOffset(0, '0', kerf.unitType),
                    ActualFingerWidth(
                            kerf.value,
                            kerf.expression,
                            kerf.unitType
                    ),
                    FingerPatternDistance(
                            pattern_distance.value,
                            pattern_distance.expression,
                            pattern_distance.unitType
                    ),
                    ActualFingerCount(
                            2, '2'
                    ),
                    joint.FingerPatternType,
                    joint.ParentPanel,
                    joint.Thickness,
                    joint.FingerWidth,
                    joint.InverseFingerPattern,
            )
            logger.debug(f'Created kerf entity: {entity}')

    def _add_joint_profile_groups(self):
        joints = self._repository.with_components(
                ActualFingerWidth, ActualFingerCount, FingerPatternDistance, FingerPatternType,
                FingerOrientation, PanelOrientation, ExtrusionDistance, ParentPanel
        ).instances

        logger.debug(f'{len(joints)} joint profile groups')
        # Group panels by Parent ID
        joints_by_parent = defaultdict(set)
        for joint in joints:
            joints_by_parent[joint.id].add(joint)

        joint_groups_by_parent = { }
        for parent_id, parent_data in joints_by_parent.items():
            joint_group = tuple(sorted(list(set([
                (
                    joint.ActualFingerWidth.expression, joint.FingerPatternDistance.expression,
                    joint.ActualFingerCount.expression, joint.FingerPatternType.finger_type,
                    joint.FingerOrientation.axis, joint.PanelOrientation.axis, joint.ExtrusionDistance.expression
                )
                for joint in parent_data
            ])
            )))
            for parent in parent_data:
                joint_groups_by_parent[parent.id] = joint_group

        for joint in joints:
            logger.debug(f'Adding joint profile to {joint.id}')
            joint.add_component(
                    JointProfile(joint_groups_by_parent[joint.id])
            )
