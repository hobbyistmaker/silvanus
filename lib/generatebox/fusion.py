import logging

from functools import partial

import adsk.core
import adsk.fusion

from .entities import AxisFlag

# noinspection SpellCheckingInspection
logger = logging.getLogger('silvanus.lib.generatebox.fusion')

yup = adsk.core.DefaultModelingOrientations.YUpModelingOrientation
zup = adsk.core.DefaultModelingOrientations.ZUpModelingOrientation

axes_selector = {
    yup: {
        (AxisFlag.Height, AxisFlag.Width):  lambda r: (r.xConstructionAxis, r.yConstructionAxis),
        (AxisFlag.Height, AxisFlag.Length): lambda r: (r.zConstructionAxis, r.yConstructionAxis),
        (AxisFlag.Width, AxisFlag.Height):  lambda r: (r.xConstructionAxis, r.yConstructionAxis),
        (AxisFlag.Length, AxisFlag.Height): lambda r: (r.zConstructionAxis, r.yConstructionAxis),
        (AxisFlag.Width, AxisFlag.Length):  lambda r: (r.yConstructionAxis, r.zConstructionAxis),
        (AxisFlag.Length, AxisFlag.Width):  lambda r: (r.yConstructionAxis, r.zConstructionAxis)
    },
    zup: {
        (AxisFlag.Height, AxisFlag.Width):  lambda r: (r.xConstructionAxis, r.zConstructionAxis),
        (AxisFlag.Height, AxisFlag.Length): lambda r: (r.yConstructionAxis, r.zConstructionAxis),
        (AxisFlag.Width, AxisFlag.Height):  lambda r: (r.xConstructionAxis, r.zConstructionAxis),
        (AxisFlag.Length, AxisFlag.Height): lambda r: (r.yConstructionAxis, r.zConstructionAxis),
        (AxisFlag.Width, AxisFlag.Length):  lambda r: (r.zConstructionAxis, r.yConstructionAxis),
        (AxisFlag.Length, AxisFlag.Width):  lambda r: (r.zConstructionAxis, r.yConstructionAxis)
    }
}

_plane_types = {
    adsk.fusion.BRepFace:          lambda b: b.body.parentComponent.sketches,
    adsk.fusion.ConstructionPlane: lambda p: p.component.sketches
}

value_converter = {
    int: adsk.core.ValueInput.createByReal,
    float: adsk.core.ValueInput.createByReal,
    str: adsk.core.ValueInput.createByString
}

def create_sketch(plane):
    """ Given a plane, returns the sketches attribute by selecting it from
        a lookup table, keyed on the plane type.
    """
    # noinspection PyUnresolvedReferences
    return _plane_types[type(plane)](plane).add(plane)


class Sketch:
    _sketch_source = {
        adsk.fusion.Sketch: lambda s: s
    }

    def __init__(self, source, *, name='', construction=False):
        self._sketch = self._sketch_source.get(type(source), create_sketch)(source)
        self._name = name
        self._sketch.name = self._name if self._name else self._sketch.name

        for line in self.lines:
            line.isConstruction = construction

        # Find SketchPoints that are connected to lines--this excludes the Origin sketch point, if it's not
        # connected to a line on the sketch.
        self._default_points = [
            point
            for points in [
                [line.startSketchPoint, line.endSketchPoint]
                for line in self.lines
            ]
            for point in points
        ]

    def __enter__(self):
        self._sketch.isComputeDeferred = True
        return self

    def __exit__(self, *args):
        self._sketch.isComputeDeferred = False

    def __getattr__(self, attr):
        if attr in self.__dict__:
            return getattr(self, attr)
        return getattr(self._sketch, attr)

    @property
    def plane(self):
        return self._sketch.referencePlane

    @property
    def lines(self):
        return self._sketch.sketchCurves.sketchLines

    @property
    def points(self):
        return self._sketch.sketchPoints

    @property
    def profile(self):
        return self._sketch.profiles.item(0)

    @property
    def features(self):
        component = self._sketch.parentComponent
        return component.features.extrudeFeatures

    @property
    def first_and_last_sketch_points(self):
        """ Returns the line-connected sketch points sorted by their (x, y, z) coordinates in the sketch.
        """
        points = sorted(self._default_points, key=lambda p: (p.geometry.x, p.geometry.y, p.geometry.z))
        return points[0], points[-1]

    @property
    def first_and_last_world_points(self):
        """ Returns the line-connected sketch points sorted by their (x, y, z) coordinates in the model world.
        """
        points = sorted(self._default_points, key=lambda p: (p.worldGeometry.x, p.worldGeometry.y, p.worldGeometry.z))
        return points[0], points[-1]

    @property
    def min_point(self):
        """ Returns the point closest to the sketch origin in the model world.
        """
        return self.first_and_last_world_points[0]

    @property
    def face_sketch_origin(self):
        return self.first_and_last_sketch_points[0]

    @property
    def world_and_sketch_min_match(self):
        """ Returns a boolean that results from comparing the minimum sketch points within the model world, and
            within the Sketch. If these match the result is true, and the sketch is oriented in a positive direction.
            If these coordinates do not match, the result is false, and the sketch is oriented in the negative
            direction.
        """
        return self.first_and_last_sketch_points[0] == self.first_and_last_world_points[0]

    def offset_point(self, start, dimensions):
        """ Return a Point3D that is offset from the start point by the given dimensions; dimensions should be an
            (x, y, z) tuple with incremental coordinates in centimeters.
        """
        length, width, height = dimensions
        factor_x = 1 if self.world_and_sketch_min_match else -1
        start_x, start_y, start_z = start.x, start.y, start.z
        return adsk.core.Point3D.create(start_x + length * factor_x, start_y + width, start_z + height)

    def offset_sketch_point(self, start, dimensions):
        """ Return a Point3D that is offset from the start point by the given dimensions; dimensions should be an
            (x, y, z) tuple with incremental coordinates in centimeters.
        """
        length, width, height = dimensions
        factor_x = 1 if self.world_and_sketch_min_match else -1
        start_x, start_y, start_z = start.geometry.x, start.geometry.y, start.geometry.z
        return adsk.core.Point3D.create(start_x + length * factor_x, start_y + width, start_z + height)

    @staticmethod
    def point_to_tuple(point):
        """ Return a tuple of (x, y, z) coordinates from the given point.
        """
        return point.x, point.y, point.z

    def extrude_non_parametric(
            self, offset, distance, direction=1
    ):
        return extrude_non_parametric_offset(
                self, offset, adsk.fusion.ExtentDirections.PositiveExtentDirection, direction, distance
        )

    def extrude_parametric(
            self, offset, distance, direction=1
    ):
        return extrude_parametric_offset(
                self, offset, adsk.fusion.ExtentDirections.PositiveExtentDirection, direction, distance
        )


class FaceProfile:

    def __init__(self, source, *, name=''):
        self._source = source
        self._name = name

    def __enter__(self):
        return self

    def __exit__(self, *_):
        pass

    @property
    def features(self):
        return self._source.bodies.item(0).parentComponent.features.extrudeFeatures

    @property
    def plane(self):
        return self._source.startFaces.item(0)

    @property
    def profile(self):
        return self._source.startFaces.item(0)

    def extrude_non_parametric(
            self, offset, distance, direction=1
    ):
        return extrude_non_parametric_offset(
                self, offset, adsk.fusion.ExtentDirections.PositiveExtentDirection, direction, distance
        )

    def extrude_reverse_non_parametric(
            self, offset, distance, direction=1
    ):
        return extrude_non_parametric_offset(
                self, offset, adsk.fusion.ExtentDirections.NegativeExtentDirection, direction, distance
        )


def extrude_simple(sketch, distance):
    distance = value_converter[type(distance)](distance)
    extrusion = sketch.features.addSimple(
            sketch.profile, distance, adsk.fusion.FeatureOperations.NewBodyFeatureOperation
    )
    return extrusion


def create_parametric_extrusion(sketch, offset, direction, multiplier, distance, operation):
    return create_extrusion(
            sketch, f'({offset.expression}) * {multiplier}',
            direction, distance.expression, operation
    )


def create_non_parametric_extrusion(sketch, offset, direction, multiplier, distance, operation):
    return create_extrusion(
            sketch, offset.value * multiplier, direction, distance.value, operation
    )


def create_extrusion(sketch, offset, direction, distance, operation):
    distance = value_converter[type(distance)](distance)
    offset = value_converter[type(offset)](offset)

    extent = adsk.fusion.DistanceExtentDefinition.create(distance)
    start = adsk.fusion.FromEntityStartDefinition.create(sketch.plane, offset)

    input_ = sketch.features.createInput(sketch.profile, operation)
    input_.setOneSideExtent(extent, direction)
    input_.startExtent = start
    return input_


def extrude_non_parametric_offset(sketch, offset, direction, multiplier, distance):
    input_ = create_non_parametric_extrusion(
            sketch, offset, direction, multiplier, distance, adsk.fusion.FeatureOperations.NewBodyFeatureOperation
    )

    return sketch.features.add(input_)


def extrude_parametric_offset(sketch, offset, direction, multiplier, distance):
    input_ = create_parametric_extrusion(
            sketch, offset, direction, multiplier, distance, adsk.fusion.FeatureOperations.NewBodyFeatureOperation
    )

    return sketch.features.add(input_)


def cut_offset(sketch, offset, direction, distance, body):
    """ Cut an extrusion using the first profile from the given sketch, using the specified offset and
        thickness/distance within the body.
    """
    input_ = create_non_parametric_extrusion(
            sketch, offset, direction, -1, distance, adsk.fusion.FeatureOperations.CutFeatureOperation
    )
    input_.participantBodies = [body]

    feature = sketch.features.add(input_)

    return feature


class PanelProfileSketch(Sketch):

    def __init__(self, *, plane_selector, transform, end, name=''):
        super().__init__(plane_selector, name=f'{name} Profile Sketch' if name else name)

        self.base_name = name
        self._transform = transform
        self._end = end
        self._profile_lines = None

    def __enter__(self):
        sketch = super().__enter__()
        self._profile_lines = self.draw_profile(sketch, self._transform, (0, 0), self._end)
        return self

    def __exit__(self, *args):
        super().__exit__(*args)

    def draw_profile(self, sketch, line_transform, origin, end):
        """ Draw a square panel profile on the given sketch, using a two point rectangle from the specified
            origin point, to the end point; with the (x, y, z) coordinates transformed by the given function.
        """
        origin_point = adsk.core.Point3D.create(*line_transform(*origin, 0))
        end_point = adsk.core.Point3D.create(*line_transform(*end, 0))
        self.draw_two_point_rectangle(sketch, origin_point, end_point)

    @staticmethod
    def draw_two_point_rectangle(sketch, origin, end):
        lines = sketch.lines.addTwoPointRectangle(origin, end)

        add_geometric_constraints(sketch)
        add_origin_constraint(sketch)
        add_profile_dimensions(sketch)

        return lines


class PanelFingerSketch(Sketch):

    def __init__(self, *, extrusion, selector, start, end, name=''):
        self._extrusion = extrusion
        self._selector = selector

        super().__init__(self.face, name=name, construction=True)

        self._start = start
        self._end = end
        self._name = name
        self.base_name = name

    def __enter__(self):
        super().__enter__()
        self.draw_finger(self._start, self._end)
        return self

    def __exit__(self, *args):
        super().__exit__(*args)

    def draw_finger(self, offset, end):
        """ Draw the finger profile on the configured face.
        """
        start_point = self.offset_sketch_point(self.min_point, (offset.value, 0, 0))
        end_point = self.offset_point(start_point, (end[0].value, end[1], 0))
        lines = self.lines.addTwoPointRectangle(start_point, end_point)

        add_line_geometric_constraints(self._sketch, lines)
        add_face_origin_constraint(self._sketch, lines, self.min_point)
        add_line_default_dimensions(self._sketch, lines)

        return lines

    def cut(self, offset, distance):
        """ Cut a profile into the extrusion.
        """
        return cut_offset(
                self,
                offset,
                adsk.fusion.ExtentDirections.NegativeExtentDirection,
                distance,
                self.face.body
        )

    @property
    def body(self):
        """ Return the body for the current extrusion object.
        """
        return self._extrusion.bodies.item(0)

    @property
    def face(self):
        """ Return the BRepFace object for the given face. This is looked up each time it is requested.
        """
        return self._selector(self.body)

    @property
    def origin_point(self):
        """ Find the point on the face closest to the world origin point.
        """
        point = sorted(self.face.vertices, key=lambda p: p.geometry.asArray())
        return point[0]


class FingerCutsPattern:

    def __init__(self, component, orientation):
        self._component = component
        self._orientation = orientation

    def copy_non_parametric(self, *, axes, cuts, distance, count):
        if count.value <= 1:
            return

        self.copy(axes=axes, cuts=cuts, distance=distance.value, count=count.value)

    def copy_parametric(self, *, axes, cuts, distance, count):
        if count.value <= 1:
            return

        self.copy(axes=axes, cuts=cuts, distance=distance.expression, count=count.expression)

    def copy(self, *, axes, cuts, distance, count):
        """ Copy an extrusion feature along a given distance with a <count> number of instances.
        """
        for cut in cuts:
            if not cut.isValid:
                return

        axis_edges = axes_selector[self._orientation][axes]

        first_edge, second_edge = axis_edges(self._component)
        patterns = self._component.features.rectangularPatternFeatures

        entities = adsk.core.ObjectCollection.create()

        for cut in cuts:
            entities.add(cut)

        pattern_input = patterns.createInput(
                entities, first_edge,
                value_converter[type(count)](count),
                value_converter[type(distance)](distance),
                adsk.fusion.PatternDistanceType.ExtentPatternDistanceType
        )
        pattern_input.patternComputeOption = adsk.fusion.PatternComputeOptions.AdjustPatternCompute
        pattern_input.directionTwoEntity = second_edge
        pattern = patterns.add(pattern_input)

        return pattern


def add_geometric_constraints(sketch):
    """ Add vertical and horizontal geometric constraints to the lines of the sketch.
    """
    add_line_geometric_constraints(sketch, sketch.lines)


def add_line_geometric_constraints(sketch, lines):
    """ Add vertical and horizontal geometric constraints to the specified lines of the sketch.
    """
    constraints = {
        True: sketch.geometricConstraints.addVertical,
        False: sketch.geometricConstraints.addHorizontal
    }

    for line in lines:
        constraints[line.startSketchPoint.geometry.x == line.endSketchPoint.geometry.x](line)


def equal_points(one, two):
    """ Check if two (x,y,z) tuples are equivalent.
    """
    return (one.x, one.y, one.z) == (two.x, two.y, two.z)


def add_origin_constraint(sketch):
    """ Add a coincident constraint to the first line that starts or ends on the sketch origin point.
    """
    add_face_origin_constraint(sketch, sketch.lines, sketch.originPoint)


def add_face_origin_constraint(sketch, lines, point):
    """ Add a coincident constraint to the first line that starts or ends on the specified sketch point.
    """
    origin = point.geometry
    for line in lines:
        if equal_points(origin, line.startSketchPoint.geometry):
            sketch.geometricConstraints.addCoincident(point, line.startSketchPoint)
            break


def add_profile_dimensions(sketch):
    """ Add dimension constraints to the first horizontal line of the given sketch.
    """
    for line in sketch.lines[:2]:
        add_distance_dimension(sketch, line)


def add_line_default_dimensions(sketch, lines):
    """ Add a distance dimension constraint to the first horizontal line of the specified lines on the sketch.
    """
    line = list(filter(lambda l: l.startSketchPoint.geometry.y == l.endSketchPoint.geometry.y, lines[:2]))[0]
    add_distance_dimension(sketch, line)


def add_distance_dimension(sketch, line):
    text_point = line.startSketchPoint.geometry
    text_point.x -= 1
    text_point.y -= 1
    sketch.sketchDimensions.addDistanceDimension(line.startSketchPoint, line.endSketchPoint, 0, text_point)
