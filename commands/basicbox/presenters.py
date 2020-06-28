from abc import ABC
from abc import abstractmethod
from collections import namedtuple

import adsk.core
import adsk.fusion
from adsk.core import ValueInput as Vi

from .fusion.sketches import Sketch, TwoPointRectangle, FingerRectangle

from .repositories import FacesRepository
from .usecases import BuildPanelUseCase

Panels = namedtuple('Panels', 'top bottom front back left right')


class BasicBoxPresenter(ABC):

    y_up = None

    @classmethod
    def create(cls, application):
        for clazz in cls.__subclasses__():
            if clazz.y_up == application.y_up:
                return clazz(application)
        return None

    def __init__(self, application):
        self.application = application
        self.component = self.application.active_component
        self.sketches = self.component.sketches
        self.parameters = self.application.parameters
        self.xy_panel = SimplePanelPresenter(self.application, self.component.xYConstructionPlane, lambda p: (p[0], p[1], 0))
        self.xz_panel = SimplePanelPresenter(self.application, self.component.xZConstructionPlane, lambda p: (p[0], -p[1], 0))
        self.yz_panel = self._yz_panel()
        self.bottom, self.left, self.back, self.right, self.front, self.top = self._setup_panels()
        self.builder = BuildPanelUseCase(self)

    @abstractmethod
    def _setup_panels(self):
        pass

    @staticmethod
    def reset_marker(presenter):
        presenter.reset_marker()

    @staticmethod
    def advance_marker(presenter):
        presenter.advance_marker()

    @abstractmethod
    def _yz_panel(self): pass


class YBasicBoxPresenter(BasicBoxPresenter):

    y_up = True

    def _yz_panel(self):
        return SimplePanelPresenter(self.application, self.component.yZConstructionPlane, lambda p: (-p[1], p[0], 0))

    def _setup_panels(self):
        return (
                self.xz_panel,
                self.yz_panel,
                self.xy_panel,
                OffsetPanelPresenter(self.application, self.yz_panel, self.parameters.length),
                OffsetPanelPresenter(self.application, self.xy_panel, self.parameters.width),
                OffsetPanelPresenter(self.application, self.xz_panel, self.parameters.height),
        )

    def build_panels(self, box):
        self.builder.build(self.bottom, box.bottom)
        self.builder.build(self.back, box.back)
        self.builder.build(self.left, box.left)
        self.builder.build(self.front, box.front)
        self.builder.build(self.right, box.right)
        self.builder.build(self.top, box.top)


class ZBasicBoxPresenter(BasicBoxPresenter):

    y_up = False

    def _yz_panel(self):
        return SimplePanelPresenter(self.application, self.component.yZConstructionPlane, lambda p: (-p[0], p[1], 0))

    def _setup_panels(self):
        return (
                self.xy_panel,
                self.yz_panel,
                OffsetPanelPresenter(self.application, self.xz_panel, self.parameters.width),
                OffsetPanelPresenter(self.application, self.yz_panel, self.parameters.length),
                self.xz_panel,
                OffsetPanelPresenter(self.application, self.xy_panel, self.parameters.height)
        )

    def build_panels(self, box):
        self.builder.build(self.bottom, box.bottom)
        self.builder.build(self.front, box.front)
        self.builder.build(self.left, box.left)
        self.builder.build(self.back, box.back)
        self.builder.build(self.right, box.right)
        self.builder.build(self.top, box.top)


SketchExtentPoints = namedtuple('SketchExtentPoints', 'start end')


class JointPresenter:

    def __init__(self, face):
        self._face = face
        self._current_face = self._face()
        self.body = self.face.body
        self.component = self.body.parentComponent
        self.patterns = self.component.features.rectangularPatternFeatures
        self.features = self.component.features.extrudeFeatures

        self.first_point = self.last_point = None

    @property
    def face(self):
        if self._current_face.isValid:
            return self._current_face
        else:
            self._current_face = self._face()
            return self._current_face

    def create(self, name, face):
        kerf_sketch = self._kerf_sketch(face, name)
        finger_sketch = self._finger_sketch(face, name)
        self._fix_kerf(face, kerf_sketch)
        self._extrude_fingers(face, finger_sketch, name)

    def _extrude_fingers(self, face, finger_sketch, name):
        primary_extrusion = self._extrude_on_primary_face(finger_sketch, face.joint.depth)

        if face.joint.sibling:
            secondary_extrusion = self._extrude_on_secondary_face(finger_sketch, face.joint.sibling)
            secondary_extrusion.name = f'{face.joint.sibling.joint.name} Extrusion'
        else:
            secondary_extrusion = None

        items = [primary_extrusion, secondary_extrusion]
        valid_items = [item for item in items if item]
        pattern = self._replicate_pattern(valid_items, face)
        primary_extrusion.name = f'{name} Extrusion'
        pattern.name = f'{name} Pattern'

    def _finger_sketch(self, face, name):
        sketch = Sketch(self.face, name=f'{name} Sketch', construction=True)
        finger_rectangle = FingerRectangle(face.joint.offset, face.joint.finger_width, face.width)
        sketch.create(finger_rectangle)
        return sketch

    def _kerf_sketch(self, face, name):
        if face.joint.kerf and face.joint.inverse:
            kerf_sketch = Sketch(self.face, name=f'{name} Kerf Sketch', construction=True)
            kerf_size = face.joint.kerf + face.joint.finger_width / 2
            kerf_rectangle = FingerRectangle(0, kerf_size, face.width)
            kerf_sketch.create(kerf_rectangle)
        else:
            kerf_sketch = None

        return kerf_sketch

    def _fix_kerf(self, face, sketch):
        if sketch and face.joint.kerf and face.joint.inverse:
            primary_extrusion = self._extrude_on_primary_face(sketch, face.joint.depth)
            primary_extrusion.name = f'{face.joint.name} Kerf Extrusion'
            if face.joint.sibling:
                secondary_extrusion = self._extrude_on_secondary_face(sketch, face.joint.sibling)
                secondary_extrusion.name = f'{face.joint.sibling.joint.name} Kerf Extrusion'
            else:
                secondary_extrusion = None

            items = [primary_extrusion, secondary_extrusion]
            valid_items = [item for item in items if item]
            pattern = self._replicate_pattern(valid_items,
                                              face,
                                              quantity=2,
                                              distance=face.length - face.joint.finger_width/2)

            pattern.name = f'{face.joint.name} Kerf Pattern'

    def _extrude_on_primary_face(self, sketch, depth):
        distance = adsk.core.ValueInput.createByString(str(-depth))
        extent = adsk.fusion.DistanceExtentDefinition.create(distance)

        input_ = self.features.createInput(
                sketch.profile, adsk.fusion.FeatureOperations.CutFeatureOperation
        )
        input_.participantBodies = [self.body]
        input_.setOneSideExtent(extent, adsk.fusion.ExtentDirections.PositiveExtentDirection)
        try:
            extrusion = self.features.add(input_)
        except RuntimeError as e:
            raise RuntimeError(f'{str(-depth)}') from e
        extrusion.extentOne.distance.expression = str(-depth)
        return extrusion

    def _extrude_on_secondary_face(self, sketch, sibling):
        distance = Vi.createByString(str(sibling.joint.depth))

        extent = adsk.fusion.DistanceExtentDefinition.create(distance)
        start = adsk.fusion.OffsetStartDefinition.create(Vi.createByString(str(-sibling.distance)))

        input_ = self.features.createInput(
                sketch.profile, adsk.fusion.FeatureOperations.CutFeatureOperation
        )
        input_.setOneSideExtent(extent, adsk.fusion.ExtentDirections.PositiveExtentDirection)
        input_.participantBodies = [self.body]
        input_.startExtent = start

        extrusion = self.features.add(input_)
        extrusion.extentOne.distance.expression = str(sibling.joint.depth)
        return extrusion

    def _replicate_pattern(self, items, face, quantity=0, distance=0):
        entities = adsk.core.ObjectCollection.create()
        for item in items:
            entities.add(item)

        distance = distance if distance else face.joint.distance + face.joint.kerf
        first_input = self._pattern_across_first_face(entities, face.joint.fingers, distance)
        pattern = self.patterns.add(first_input)
        pattern.quantityOne.expression = str(face.joint.fingers) if not quantity else f'{quantity}'
        return pattern

    def _pattern_across_first_face(self, items, fingers, distance):
        first_edge, second_edge = self.pattern_edges
        pattern_input = self.patterns.createInput(items, first_edge,
                                                  adsk.core.ValueInput.createByString(str(fingers)),
                                                  adsk.core.ValueInput.createByString(str(distance)),
                                                  adsk.fusion.PatternDistanceType.ExtentPatternDistanceType)
        pattern_input.patternComputeOption = adsk.fusion.PatternComputeOptions.AdjustPatternCompute
        pattern_input.directionTwoEntity = second_edge
        return pattern_input

    @property
    def edges(self):
        return [edge for edge in self.origin_point.edges if edge in self.face.edges]

    @property
    def long_edge(self):
        return sorted(self.edges, key=lambda edge: edge.length, reverse=True)[0]

    @property
    def short_edge(self):
        return sorted(self.edges, key=lambda edge: edge.length)[0]

    @property
    def origin_point(self):
        point = sorted(self.face.vertices, key=lambda p: p.geometry.asArray())[0]
        return point

    @property
    def pattern_edges(self):
        return self.long_edge, self.far_edge

    @property
    def far_edge(self):
        edge = [edge for edge in self.origin_point.edges if edge not in self.face.edges][0]
        return edge


class PanelPresenter:

    def __init__(self, app, reference):
        self.app = app
        self.reference = reference
        self.component = self.reference.component
        self.features = self.component.features.extrudeFeatures
        self._name = ''
        self._start = (0, 0, 0)
        self._end = None
        self.length = None
        self.width = None
        self.kerf = None
        self._thickness = None
        self.extrusion = None
        self.faces_repository = FacesRepository(self.app)
        self.body_marker = 0
        self.current_marker = 0

    @property
    def name(self):
        return self._name

    def create(self, joints):
        start_marker = self.app.timeline.markerPosition

        self._pre_create()

        self.extrusion = self._extrude()
        self.extrusion.name = f'{self.name} Panel Extrusion'
        body = self.extrusion.bodies.item(0)
        body.name = f'{self.name} Panel Body'
        self.body_marker = self.app.timeline.markerPosition

        self._post_create(joints, body)

        end_marker = self.app.timeline.markerPosition
        if end_marker - start_marker > 1:
            timeline_group = self.app.timeline.timelineGroups.add(start_marker, end_marker - 1)
            timeline_group.name = f'{self.name} Panel Group'

    def reset_marker(self):
        self.current_marker = self.app.timeline.markerPosition
        self.app.timeline.markerPosition = self.body_marker

    def advance_marker(self):
        current_marker = self.app.timeline.markerPosition
        self.app.timeline.markerPosition = self.current_marker + (current_marker - self.body_marker)

    @name.setter
    def name(self, value):
        self._name = value

    @property
    def start(self):
        return self._start

    @start.setter
    def start(self, value):
        self._start = value

    @property
    def end(self):
        return self._end

    @end.setter
    def end(self, value):
        self._end = value

    @property
    def thickness(self):
        return self._thickness

    @thickness.setter
    def thickness(self, value):
        self._thickness = value

    def _extrude(self): pass

    def _pre_create(self): pass

    def _post_create(self, use_case, body): pass


class SimplePanelPresenter(PanelPresenter):

    def __init__(self, app, reference, point_func):
        super().__init__(app, PanelSketchPresenter(reference, point_func))

    def _pre_create(self):
        self.draw()

    def draw(self):
        self.reference.draw(self.name, self.start, self.end, dimensions=(self.length, self.width))

    def _post_create(self, joint_faces, body):
        presenter = FacesPresenter(self.faces_repository.faces(body))
        joint_faces.create(presenter)

    def _extrude(self):
        distance = Vi.createByString(str(self.thickness))
        extrusion = self.features.addSimple(self.reference.profile, distance,
                                            adsk.fusion.FeatureOperations.NewBodyFeatureOperation)
        extrusion.extentOne.distance.expression = str(self.thickness)
        return extrusion

    @property
    def profile(self):
        return self.extrusion.startFaces[0]


class OffsetPanelPresenter(PanelPresenter):

    def __init__(self, app, reference, offset):
        super().__init__(app, reference)
        self.offset = offset

    def _extrude(self):
        is_panel = isinstance(self.reference, PanelPresenter)
        kerf_offset = self.offset + self.kerf if self.kerf else self.offset
        offset = -kerf_offset if is_panel else kerf_offset - self.thickness
        distance = Vi.createByString(self.thickness.expression)
        extent = adsk.fusion.DistanceExtentDefinition.create(distance)

        start = adsk.fusion.OffsetStartDefinition.create(Vi.createByString(offset.expression))

        input_ = self.features.createInput(
                self.reference.profile, adsk.fusion.FeatureOperations.NewBodyFeatureOperation
        )
        input_.setOneSideExtent(extent, adsk.fusion.ExtentDirections.PositiveExtentDirection)
        input_.startExtent = start

        extrusion = self.features.add(input_)
        extrusion.extentOne.distance.expression = str(self.thickness)
        return extrusion


class PanelSketchPresenter:

    def __init__(self, plane, point_func):
        self.point_func = point_func
        self.plane = plane
        self.component = self.plane.component
        self.sketch = None

    def draw(self, name, start, end, dimensions):
        self.sketch = Sketch(self.plane, name=f'{name} Panel Sketch')

        rectangle = TwoPointRectangle(self.point_func(start), self.point_func(end))
        rectangle.dimensions = dimensions

        return self.sketch.create(rectangle)

    @property
    def profile(self):
        return self.sketch.profile


class FacesPresenter:

    def __init__(self, faces):
        self.faces = faces

    def __getattr__(self, item):
        if item in ['left', 'right', 'front', 'back', 'top', 'bottom']:
            return JointPresenter(lambda: getattr(self.faces, item, None))