import logging
from collections import defaultdict
from collections import namedtuple
from pprint import pformat

import adsk.core

from .ecs import Process
from .entities import ActualFingerCount
from .entities import ActualFingerWidth
from .entities import AxisFlag
from .entities import AxisGroup
from .entities import ExtrusionDistance
from .entities import FingerOffset
from .entities import FingerOrientation
from .entities import FingerPatternDistance
from .entities import FingerPatternType
from .entities import JointName
from .entities import JointPanelOffset
from .entities import JointPatternDistance
from .entities import JointProfile
from .entities import JointThickness
from .entities import KerfJoint
from .entities import PanelJoint
from .entities import PanelName
from .entities import PanelOffset
from .entities import PanelOrientation
from .entities import PanelProfile
from .entities import ParentPanel
from .entities import Thickness
from .fusion import FaceProfile
from .fusion import FingerCutsPattern
from .fusion import PanelFingerSketch
from .fusion import PanelProfileSketch

# noinspection SpellCheckingInspection
logger = logging.getLogger('silvanus.lib.generatebox.render')


class RenderSystem(Process):

    def __init__(self, root, orientation):
        self._root = root
        self._orientation = orientation

        self.groups = defaultdict(lambda: AxisGroup())

    def process(self):
        self._define_group_profiles(self.groups, KerfJoint)
        self._define_group_profiles(self.groups, PanelJoint)
        self._render_profile_groups(self.groups)
        self.groups.clear()

    def _define_group_profiles(self, groups, filter):
        PanelConfig = namedtuple('PanelConfig', [
            'PanelProfile', 'PanelOrientation', 'PanelName', 'ExtrusionDistance', 'PanelOffset', 'ParentPanel'
        ])
        JointConfig = namedtuple('JointConfig', [
            'JointPatternDistance', 'JointPanelOffset', 'JointThickness', 'FingerOrientation',
            'FingerOffset', 'ActualFingerWidth', 'FingerPatternDistance', 'ActualFingerCount', 'FingerPatternType',
            'PanelThickness', 'JointName'
        ])
        components = (
            PanelProfile, PanelOrientation, PanelName, ExtrusionDistance, PanelOffset,
            JointPatternDistance, JointPanelOffset, JointName, JointThickness, FingerOrientation,
            FingerOffset, ActualFingerWidth, FingerPatternDistance, ActualFingerCount, FingerPatternType,
            ParentPanel, JointProfile, Thickness, filter
        )
        entities = self._repository.with_components(*components).instances

        for component in components:
            renderables = self._repository.with_components(component).instances
            logger.debug(f'{len(renderables)} {component.__name__}')

        logger.debug(f'{len(entities)} group profiles to define')
        for entity in entities:
            panel = PanelConfig(
                    entity.PanelProfile, entity.PanelOrientation, entity.PanelName, entity.ExtrusionDistance,
                    entity.PanelOffset, entity.ParentPanel
            )
            joint = JointConfig(
                    entity.JointPatternDistance, entity.JointPanelOffset, entity.JointThickness,
                    entity.FingerOrientation, entity.FingerOffset, entity.ActualFingerWidth,
                    entity.FingerPatternDistance, entity.ActualFingerCount, entity.FingerPatternType,
                    entity.Thickness, entity.JointName
            )

            orientation = groups[entity.PanelOrientation]

            panel_profile = orientation.profiles[entity.PanelProfile]
            panel_profile.names.add(entity.PanelName.value)

            distance = panel_profile.panels[entity.ExtrusionDistance]
            distance.panel = panel
            distance.extrusions.add(panel)

            joint_profile = distance.joint_types[filter].joint_orientations[entity.FingerOrientation].joint_profiles[
                entity.JointProfile]
            joint_profile.names.add(entity.JointName.value)
            joint_profile.joint = joint
            joint_profile.joint_extrusions.add(joint)

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

    def _render_profile_groups(self, groups):
        for axis_group_id, axis_group in groups.items():
            for profile, profile_data in axis_group.profiles.items():
                end = (profile.length.value, profile.width.value)
                name = '-'.join(profile_data.names)
                panel_groups = profile_data.panels

                transform = self._select_group_transform(axis_group_id)
                plane_selector = self._select_group_plane(axis_group_id)

                app = adsk.core.Application.get()
                timeline = app.activeProduct.timeline
                start = timeline.markerPosition

                with PanelProfileSketch(
                        plane_selector=plane_selector,
                        transform=transform,
                        end=end,
                        name=name
                ) as profile_sketch:
                    for panel_id, panel_data in panel_groups.items():
                        self._extrude_profiles(name, profile_sketch, panel_id, panel_data)

                end = timeline.markerPosition

                if (end - 1) - start <= 1:
                    continue

                group = timeline.timelineGroups.add(start, end - 1)
                group.name = f'{name} Panel Group'

    def _extrude_profiles(self, group_name, profile, thickness, panel_data):
        _render_func = {
            False: self._render_single_extrusion,
            True:  self._render_copy_extrusion
        }
        current_reference = profile

        for id_, panel in enumerate(sorted(panel_data.extrusions, key=lambda p: p.PanelOffset.value)):
            func = _render_func[bool(id_)]
            # noinspection PyArgumentList
            current_reference = func(group_name, current_reference, thickness, panel, panel_data)

    def _render_single_extrusion(self, group_name, profile, thickness, panel, panel_data):
        logger.debug(f'{profile}')
        extrusion = profile.extrude(panel.PanelOffset, thickness)
        extrusion.name = f'{panel.PanelName.value} Panel Extrusion'
        body = extrusion.bodies.item(0)
        body.name = f'{panel.PanelName.value} Panel Body'

        names = {
            KerfJoint:  'Kerf',
            PanelJoint: 'Finger'
        }
        profiles = []

        for joint_type_id, joint_type in panel_data.joint_types.items():
            profiles.append((
                joint_type_id, self._render_finger_profiles(group_name, extrusion, joint_type, names[joint_type_id])
            ))

        for joint_type, profile in profiles:
            for sketch, joints in profile:
                features = []

                for joint in joints.joint_extrusions:
                    feature = sketch.cut(joint.JointPanelOffset.value, joint.JointThickness.value)
                    feature.name = f'{panel.PanelName.value} {joint.JointName.value} {names[joint_type]} Extrusion'
                    features.append(feature)

                replicator = FingerCutsPattern(self._root, self._orientation)
                joint = joints.joint
                distance = joint.FingerPatternDistance
                count = joint.ActualFingerCount
                joint_names = f'{("-").join(joints.names)}'
                axes = (panel_data.panel.PanelOrientation.axis, joint.FingerOrientation.axis)

                pattern = replicator.copy(axes=axes, cuts=features, distance=distance, count=count)
                if pattern:
                    pattern.name = f'{panel.PanelName.value} {joint_names} {names[joint_type]} Pattern'

        return extrusion

    @staticmethod
    def _render_copy_extrusion(group_name, reference, thickness, panel, _):
        with FaceProfile(reference, name='') as profile:
            extrusion = profile.extrude_reverse(panel.PanelOffset, thickness, -1)
            extrusion.name = f'{panel.PanelName.value} Panel Extrusion'
            body = extrusion.bodies.item(0)
            body.name = f'{panel.PanelName.value} Panel Body'

        return reference

    def _render_finger_profiles(self, group_name, extrusion, panel_data, name):
        face_selectors = {
            0: {
                AxisFlag.Width:  lambda body: sorted(body.faces, key=lambda f: f.centroid.z)[0],
                AxisFlag.Length: lambda body: sorted(body.faces, key=lambda f: f.centroid.x)[0],
                AxisFlag.Height: lambda body: sorted(body.faces, key=lambda f: f.centroid.y)[0]
            },
            1: {
                AxisFlag.Width:  lambda body: sorted(body.faces, key=lambda f: f.centroid.y)[0],
                AxisFlag.Length: lambda body: sorted(body.faces, key=lambda f: f.centroid.x)[0],
                AxisFlag.Height: lambda body: sorted(body.faces, key=lambda f: f.centroid.z)[0]
            }
        }

        finger_profiles = []

        for orientation_id, orientation_data in panel_data.joint_orientations.items():
            face_selector = face_selectors[self._orientation][orientation_id.axis]

            logger.debug(f'Joint profiles: {pformat(orientation_data.joint_profiles, width=1000)}')
            for profile_group_id, profile_group_data in orientation_data.joint_profiles.items():
                joint_names = f'{"-".join(profile_group_data.names)}'
                profile_name = f'{group_name} {joint_names} {name} Sketch'
                finger_offset = profile_group_data.joint.FingerOffset
                finger_width = profile_group_data.joint.ActualFingerWidth
                panel_thickness = profile_group_data.joint.PanelThickness

                with PanelFingerSketch(
                        extrusion=extrusion,
                        selector=face_selector,
                        name=profile_name,
                        start=finger_offset,
                        end=(finger_width, panel_thickness.value)
                ) as sketch:
                    finger_profiles.append((sketch, profile_group_data))

        return finger_profiles
