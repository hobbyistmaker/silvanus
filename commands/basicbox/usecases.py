import math

from .entities import Box
from .entities import Joint, JointSibling
from .entities import Parameters
from .entities import JointParameters, InverseJointParameters, KerfJointParameters, InverseKerfJointParameters


class CreateFaceJointsUseCase:

    def __init__(self, box_presenter, data):
        self.data = data
        self.box_presenter = box_presenter

    def create(self, box_presenter):
        self._create_front_back(self.data.front, self.data.back, box_presenter)
        self._create_left_right(self.data.left, self.data.right, box_presenter)
        self._create_top_bottom(self.data.top, self.data.bottom, box_presenter)

    def _create_front_back(self, front, back, box_presenter):
        if front.joint:
            self._create_joint(front, box_presenter.front)
        if back.joint and not front.joint == back.joint:
            self._create_joint(back, box_presenter.back)

    def _create_left_right(self, left, right, box_presenter):
        if left.joint:
            self._create_joint(left, box_presenter.left)
        if right.joint and not left.joint == right.joint:
            self._create_joint(right, box_presenter.right)

    def _create_top_bottom(self, top, bottom, box_presenter):
        if bottom.joint:
            self._create_joint(bottom, box_presenter.bottom)
        if top.joint and not bottom.joint == top.joint:
            self._create_joint(top, box_presenter.top)

    @staticmethod
    def _create_joint(face, box_presenter):
        if face and face.joint:
            box_presenter.create(f'{face.joint.name}', face)


class SetupJointUseCase:

    def __init__(self, box):
        self.left, self.right = box.left, box.right
        self.top, self.bottom = box.top, box.bottom
        self.front, self.back = box.front, box.back
        self.kerf = box.kerf

    @staticmethod
    def _left_right_joints(panel, joint, left, right, secondary):
        left_joint = joint()
        right_joint = joint()

        if left:
            panel.left.joint = left_joint
            panel.left.joint.name = f'{panel.left.joint.name} Left Joint'
            panel.left.joint.depth = left.thickness
            panel.left.joint.sibling = JointSibling(secondary, right_joint) if right else None
            panel.left.joint.kerf = panel.kerf
        if right:
            panel.right.joint = right_joint
            panel.right.joint.name = f'{panel.right.joint.name} Right Joint'
            panel.right.joint.depth = right.thickness
            panel.right.joint.sibling = JointSibling(secondary, left_joint) if left else None
            panel.right.joint.kerf = panel.kerf

    @staticmethod
    def _front_back_joints(panel, joint, front, back, secondary):
        front_joint = joint()
        back_joint = joint()

        if front:
            panel.front.joint = front_joint
            panel.front.joint.name = f'{panel.front.joint.name} Front Joint'
            panel.front.joint.depth = front.thickness
            panel.front.joint.sibling = JointSibling(secondary, back_joint) if back else None
            panel.front.joint.kerf = panel.kerf
        if back:
            panel.back.joint = back_joint
            panel.back.joint.name = f'{panel.back.joint.name} Back Joint'
            panel.back.joint.depth = back.thickness
            panel.back.joint.sibling = JointSibling(secondary, front_joint) if front else None
            panel.back.joint.kerf = panel.kerf

    @staticmethod
    def _top_bottom_joints(panel, joint, top, bottom, secondary):
        top_joint = joint()
        bottom_joint = joint()

        if top:
            panel.top.joint = top_joint
            panel.top.joint.name = f'{panel.top.joint.name} Top Joint'
            panel.top.joint.depth = top.thickness
            panel.top.joint.sibling = JointSibling(secondary, bottom_joint) if bottom else None
            panel.top.joint.kerf = panel.kerf
        if bottom:
            panel.bottom.joint = bottom_joint
            panel.bottom.joint.name = f'{panel.bottom.joint.name} Bottom Joint'
            panel.bottom.joint.depth = bottom.thickness
            panel.bottom.joint.sibling = JointSibling(secondary, top_joint) if top else None
            panel.bottom.joint.kerf = panel.kerf

    @staticmethod
    def _setup_length_joint(add_joint, panel, *, tangent1=None, tangent2=None, inverse=False):
        t1_length = panel.length - tangent1.thickness if tangent1 else panel.length
        t2_length = t1_length - tangent2.thickness if tangent2 else t1_length
        return add_joint.build(name=panel.name, length=t2_length, inverse=inverse)

    @staticmethod
    def _setup_width_joint(add_joint, panel, *, tangent1=None, tangent2=None, inverse=False):
        t1_width = panel.width - tangent1.thickness if tangent1 else panel.width
        t2_width = t1_width - tangent2.thickness if tangent2 else t1_width
        return add_joint.build(name=panel.name, length=t2_width, inverse=inverse)

    def setup_top_bottom(self, add_joint, panel):
        def length_face(): return self._setup_length_joint(add_joint, panel)
        def width_face(): return self._setup_width_joint(add_joint, panel)
        self._front_back_joints(panel, length_face, self.front, self.back, panel.width + panel.kerf)
        self._left_right_joints(panel, width_face, self.left, self.right, panel.length + panel.kerf)

    def setup_left_right(self, add_joint, panel):
        def length_face(): return self._setup_length_joint(add_joint, panel)
        def width_face(): return self._setup_width_joint(add_joint, panel, inverse=True)
        self._front_back_joints(panel, length_face, self.front, self.back, panel.width + panel.kerf)
        self._top_bottom_joints(panel, width_face, self.top, self.bottom, panel.length + panel.kerf)

    def setup_front_back(self, add_joint, panel):
        def length_face(): return self._setup_length_joint(add_joint, panel, inverse=True)
        def width_face(): return self._setup_width_joint(add_joint, panel, inverse=True)
        self._left_right_joints(panel, width_face, self.left, self.right, panel.length + panel.kerf)
        self._top_bottom_joints(panel, length_face, self.top, self.bottom, panel.width + panel.kerf)


class DefineBoxUseCase:

    def __init__(self, presenter, parameters):
        self.presenter = presenter
        self.parameters = parameters

        self.length = self.parameters.length
        self.width = self.parameters.width
        self.height = self.parameters.height
        self.thickness = self.parameters.thickness
        self.finger_width = self.parameters.finger_width
        self.kerf = self.parameters.kerf


    def build(self):
        box = self._setup_box()

        self._setup_joints(box, self.finger_width, self.kerf)

        self.presenter.build_panels(box)

    @staticmethod
    def _setup_joints(box, finger_width, kerf):
        joints = DefineJointedFaceUseCase(finger_width, kerf)
        setup = SetupJointUseCase(box)
        joints.add(setup.setup_top_bottom, [box.top, box.bottom])
        joints.add(setup.setup_front_back, [box.front, box.back])
        joints.add(setup.setup_left_right, [box.left, box.right])

    def _setup_box(self):
        box = Box(self.length, self.width, self.height, self.thickness, self.kerf)
        box.top.enabled = bool(self.parameters.top_enabled)
        box.top.thickness = self.parameters.top_thickness or box.top.thickness
        box.bottom.enabled = bool(self.parameters.bottom_enabled)
        box.bottom.thickness = self.parameters.bottom_thickness or box.bottom.thickness
        box.left.enabled = bool(self.parameters.left_enabled)
        box.left.thickness = self.parameters.left_thickness or box.left.thickness
        box.right.enabled = bool(self.parameters.right_enabled)
        box.right.thickness = self.parameters.right_thickness or box.right.thickness
        box.front.enabled = bool(self.parameters.front_enabled)
        box.front.thickness = self.parameters.front_thickness or box.front.thickness
        box.back.enabled = bool(self.parameters.back_enabled)
        box.back.thickness = self.parameters.back_thickness or box.back.thickness
        return box


class DefineJointedFaceUseCase:

    def __init__(self, finger_width, kerf):
        self.finger_width = finger_width
        self.kerf = kerf

    def add(self, func, panels):
        for panel in panels:
            func(self, panel)

    def build(self, *, name, length, inverse=False):
        if self.kerf:
            return self._kerf_joint(name, length, inverse)
        else:
            return self._joint(name, length, inverse)

    def _joint(self, name, length, inverse):
        if self.kerf:
            if inverse:
                parameters = InverseKerfJointParameters(length, self.finger_width, self.kerf)
            else:
                parameters = KerfJointParameters(length, self.finger_width, self.kerf)
        else:
            if inverse:
                parameters = InverseJointParameters(length, self.finger_width)
            else:
                parameters = JointParameters(length, self.finger_width)

        return Joint(name,
                     parameters.actual_width,
                     parameters.actual_fingers,
                     parameters.distance,
                     parameters.offset,
                     inverse)

    def _kerf_joint(self, name, length, inverse):
        joint = self._joint(name, length, inverse)
        joint.finger_width -= self.kerf
        joint.distance -= self.kerf
        joint.offset += self.kerf
        return joint


class BuildPanelUseCase:

    def __init__(self, presenter):
        self.presenter = presenter

    def build(self, panel, data):
        if data and panel:
            joints = CreateFaceJointsUseCase(self.presenter, data)
            panel.name = data.name
            panel.start = data.start
            panel.end = data.end
            panel.length = data.length + data.kerf
            panel.width = data.width + data.kerf
            panel.thickness = data.thickness
            panel.kerf = data.kerf
            panel.create(joints)


class DefineParametersUseCase:

    def __init__(self, fusion_parameters):
        self.fusion_parameters = fusion_parameters

    def build(self):
        parameters = Parameters()

        parameters.add('length', *self.fusion_parameters.length)
        parameters.add('width', *self.fusion_parameters.width)
        parameters.add('height', *self.fusion_parameters.height)
        parameters.add('thickness', *self.fusion_parameters.thickness)
        parameters.add('finger_width', *self.fusion_parameters.finger_width)
        parameters.add('kerf', *self.fusion_parameters.kerf)

        options = self.fusion_parameters.options

        for item in ['top', 'bottom', 'left', 'right', 'front', 'back']:
            parameters.add(f'{item}_enabled', *options[item]['enable'])
            parameters.add(f'{item}_thickness', *options[item]['thickness'])

        return parameters
