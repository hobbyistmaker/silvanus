from collections import namedtuple

import adsk.core
import adsk.fusion

from .profile import Profile


class Sketch:

    def __init__(self, plane, *, name=None, construction=False):
        self.plane = plane
        self.component = self._component(self.plane)
        self.sketch = self.component.sketches.add(self.plane)
        self.construction = construction

        if name:
            self.sketch.name = name

    def create(self, drawing):
        self.sketch.isComputeDeferred = True

        if self.construction:
            self.switch_profile_to_construction()

        drawing = drawing.create(self)

        self.sketch.isComputeDeferred = False
        return drawing

    def offset_from_point(self, start, point):
        start = start if isinstance(start, adsk.core.Point3D) else start
        factor = -1 if self.is_reversed else 1
        return adsk.core.Point3D.create(start.x + point.x * factor,
                                        start.y + point.y,
                                        start.z + point.z)

    def offset_from_origin(self, point):
        return self.offset_from_point(ModelPoint(0, 0, 0), point)

    def switch_profile_to_construction(self):
        for line in self.sketch.sketchCurves.sketchLines:
            line.isConstruction = True

    @staticmethod
    def _component(item):
        if isinstance(item, adsk.fusion.ConstructionPlane):
            return item.component
        elif isinstance(item, adsk.fusion.BRepFace):
            return item.body.parentComponent
        raise NotImplemented

    @property
    def lines(self):
        return self.sketch.sketchCurves.sketchLines

    @property
    def profile(self):
        return self.sketch.profiles.item(0)

    @property
    def is_deferred(self):
        return self.sketch.isComputeDeferred

    @is_deferred.setter
    def is_deferred(self, value):
        self.sketch.isComputeDeferred = value

    @property
    def surface(self):
        return self.sketch

    @property
    def is_reversed(self):
        first, last = self._first_and_last_points()
        fp = (first.worldGeometry.x, first.worldGeometry.y, first.worldGeometry.z)
        lp = (last.worldGeometry.x, last.worldGeometry.y, last.worldGeometry.z)

        is_vertical = first.geometry.x < 0
        return (lp < fp) or (is_vertical and (fp < lp))

    def _first_and_last_points(self):
        sorted_points = sorted(self.connected_points, key=sketch_point_key)
        return (sorted_points[0], sorted_points[-1]) if sorted_points else (None, None)

    @property
    def connected_points(self):
        return [point for point in self.sketch.sketchPoints if point.connectedEntities]


class TwoPointRectangle:

    def __init__(self, start, end):
        self.sketch = None
        self.start = adsk.core.Point3D.create(*start)
        self.end = adsk.core.Point3D.create(*end)
        self.lines = None
        self.dimensions = None

    def create(self, sketch):
        self.sketch = sketch
        self.lines = sketch.lines.addTwoPointRectangle(self.start, self.end)

        if self.dimensions:
            self.add_dimension_constraints(self.dimensions)
        return self

    def add_dimension_constraints(self, dimensions):
        _add_panel_dimension_constraints(self.lines, dimensions=dimensions)


class FingerRectangle:

    def __init__(self, offset, width, thickness):
        self.width = width
        self.offset = offset
        self.thickness = thickness

    def create(self, sketch):
        # sketch.is_deferred = True

        start = sketch.offset_from_origin(ModelPoint(float(self.offset), 0, 0))
        end = sketch.offset_from_point(start, ModelPoint(float(self.width), float(self.thickness), 0))

        lines = sketch.lines.addTwoPointRectangle(start, end)
        _add_geometric_constraints(lines)
        dimensions = _add_finger_dimension_constraint(lines, dimension=self.width)
        sketch_lines = Profile(sketch.surface)
        finger_lines = Profile(sketch.surface, parent=sketch_lines, lines=lines, dimensions=dimensions)

        finger_line, sketch_line = _bottom_right_collinear_constraint(finger_lines, sketch_lines)
        _top_left_collinear_constraint(finger_lines, sketch_lines)

        if self.offset > 0:
            # offset_dimension = _add_offset_dimension(sketch_line, finger_line)
            offset_dimension = _add_origin_offset_dimension(finger_line)
            if offset_dimension:
                offset_eq = self.offset and self.offset == self.width
                offset_val = self.offset.expression if self.offset else self.width
                offset_dimension.parameter.expression = dimensions.parameter.name if offset_eq else offset_val
        else:
            _add_origin_constraint(finger_line)

        sketch.is_deferred = False

        return finger_lines


def two_point_rectangle(sketch, start, end, constrain_origin=False):
    lines = _draw_rectangle(sketch, start, end)
    _add_geometric_constraints(lines)

    if constrain_origin:
        _add_origin_constraint(lines[0])

    return lines


def _add_finger_dimension_constraint(lines, dimension=None):
    is_vertical = _is_vertical(lines[0])
    horizontal = lines[1] if is_vertical else lines[0]
    vertical = lines[0] if is_vertical else lines[1]

    constraint = (
            _add_vertical_line_dimension(vertical)
            if is_vertical else
            _add_horizontal_line_dimension(horizontal)
    )
    if dimension:
        constraint.parameter.expression = dimension.expression
    return constraint


def _bottom_right_collinear_constraint(finger_lines, sketch_lines):
    sketch_line, finger_line = (
            (sketch_lines.right, finger_lines.right)
            if sketch_lines.is_vertical
            else (sketch_lines.bottom, finger_lines.bottom)
    )
    _collinear_constrain_lines(sketch_line, finger_line)
    return finger_line, sketch_line


def _top_left_collinear_constraint(finger_lines, sketch_lines):
    sketch_line, finger_line = (
            (sketch_lines.left, finger_lines.left)
            if sketch_lines.is_vertical
            else (sketch_lines.top, finger_lines.top)
    )
    _collinear_constrain_lines(sketch_line, finger_line)
    return finger_line, sketch_line


def _add_offset_dimension(sketch_line, finger_line):
    sketch_point = sketch_line.endSketchPoint if _line_is_reversed(sketch_line) else sketch_line.startSketchPoint
    finger_point = finger_line.startSketchPoint
    return (
            _add_vertical_dimension_two_points(sketch_point, finger_point)
            if _is_vertical(finger_line)
            else _add_horizontal_dimension_two_points(sketch_point, finger_point)
    )


def _add_origin_offset_dimension(finger_line):
    origin = finger_line.startSketchPoint.parentSketch.originPoint
    finger_point = finger_line.startSketchPoint
    return (
            _add_vertical_dimension_two_points(origin, finger_point)
            if _is_vertical(finger_line) else
            _add_horizontal_dimension_two_points(origin, finger_point)
    )


def _draw_rectangle(sketch, start, end):
    lines = sketch.sketchCurves.sketchLines
    rectangle_end, rectangle_start = _define_points(sketch, start, end)
    try:
        lines = lines.addTwoPointRectangle(rectangle_start, rectangle_end)
    except:
        raise LookupError(f'{rectangle_start.asArray()} {rectangle_end.asArray()}')
    return lines


def _add_geometric_constraints(lines):
    sketch = lines[0].parentSketch

    horizontal1, horizontal2 = (lines[1], lines[3]) if _is_vertical(lines[0]) else (lines[0], lines[2])
    vertical1, vertical2 = (lines[1], lines[3]) if _is_vertical(lines[1]) else (lines[0], lines[2])

    sketch.geometricConstraints.addHorizontal(horizontal1)
    sketch.geometricConstraints.addHorizontal(horizontal2)
    sketch.geometricConstraints.addVertical(vertical1)
    sketch.geometricConstraints.addVertical(vertical2)


def _add_origin_constraint(line):
    sketch = line.parentSketch
    start = line.startSketchPoint
    sketch.geometricConstraints.addCoincident(start, sketch.originPoint)


def _is_vertical(line):
    start = line.startSketchPoint.geometry
    end = line.endSketchPoint.geometry
    return True if (start.x == end.x) else False


def _add_vertical_line_dimension(line):
    orientation = adsk.fusion.DimensionOrientations.VerticalDimensionOrientation
    return _add_line_dimension(line, orientation)


def _add_horizontal_line_dimension(line):
    orientation = adsk.fusion.DimensionOrientations.HorizontalDimensionOrientation
    return _add_line_dimension(line, orientation)


def _collinear_constrain_lines(line1, line2):
    sketch = line1.parentSketch
    sketch.geometricConstraints.addCollinear(line1, line2)


def _line_is_reversed(line):
    first, last = line.startSketchPoint, line.endSketchPoint
    return _is_reversed(first, last)


def _add_horizontal_dimension_two_points(start, end):
    orientation = adsk.fusion.DimensionOrientations.HorizontalDimensionOrientation
    sketch = end.parentSketch
    return _add_dimension(start, end, sketch, orientation)


def _add_vertical_dimension_two_points(start, end):
    orientation = adsk.fusion.DimensionOrientations.VerticalDimensionOrientation
    sketch = end.parentSketch
    return _add_dimension(start, end, sketch, orientation)


def _define_points(sketch, start, end):
    p3d_start = start if _check_point3d(start) else adsk.core.Point3D.create(*start)
    p3d_end = end if _check_point3d(end) else adsk.core.Point3D.create(*end)
    rectangle_start = sketch.modelToSketchSpace(p3d_start) if isinstance(start, ModelPoint) else p3d_start
    rectangle_end = sketch.modelToSketchSpace(p3d_end) if isinstance(end, ModelPoint) else p3d_end
    return rectangle_end, rectangle_start


def _add_line_dimension(line, orientation):
    sketch = line.parentSketch
    start, end = line.startSketchPoint, line.endSketchPoint
    return _add_dimension(start, end, sketch, orientation)


def _add_dimension(start, end, sketch, orientation):
    if isinstance(start, adsk.core.Point3D):
        geometry = start
    else:
        geometry = start.geometry
    label = (geometry.x - 1, geometry.y - 1, 0)
    return sketch.sketchDimensions.addDistanceDimension(
            start, end, orientation,
            adsk.core.Point3D.create(*label)
    )


def _is_reversed(first, last):
    fp = (first.worldGeometry.x, first.worldGeometry.y, first.worldGeometry.z)
    lp = (last.worldGeometry.x, last.worldGeometry.y, last.worldGeometry.z)

    is_vertical = first.geometry.x < 0
    return True if (lp < fp) or (is_vertical and (fp < lp)) else False


def _check_sketch_point(point):
    return True if isinstance(point, adsk.fusion.SketchPoint) else False


def _check_point3d(point):
    return True if isinstance(point, adsk.core.Point3D) else _check_sketch_point(point)


def sketch_point_key(point):
    geometry = point.geometry
    return geometry.x, geometry.y, geometry.z


def draw_finger_rectangle(sketch, start, end, width=None, offset=None, use_origin=False):
    lines = two_point_rectangle(sketch, start, end, constrain_origin=False)
    dimensions = _add_finger_dimension_constraint(lines, dimension=width)
    sketch.isComputeDeferred = False
    sketch_lines = Profile(sketch)
    finger_lines = Profile(sketch, parent=sketch_lines, lines=lines, dimensions=dimensions)

    finger_line, sketch_line = _bottom_right_collinear_constraint(finger_lines, sketch_lines)
    _top_left_collinear_constraint(finger_lines, sketch_lines)

    if offset > 0:
        offset_dimension = _add_offset_dimension(sketch_line, finger_line)
        if offset_dimension:
            offset_eq = offset and offset == width
            offset_val = offset.expression if offset else width
            offset_dimension.parameter.expression = dimensions.parameter.name if offset_eq else offset_val
    else:
        _add_origin_constraint(finger_line)
    return finger_lines


def _add_panel_dimension_constraints(lines, dimensions=None):
    horizontal_dimension, vertical_dimension = dimensions
    is_vertical = _is_vertical(lines[0])

    horizontal = lines[1] if is_vertical else lines[0]
    vertical = lines[0] if is_vertical else lines[1]

    x = _add_horizontal_line_dimension(horizontal)
    y = _add_vertical_line_dimension(vertical)

    if horizontal:
        x.parameter.expression = horizontal_dimension.expression
    if vertical:
        y.parameter.expression = vertical_dimension.expression

    _add_origin_constraint(horizontal)
    _add_geometric_constraints(lines)

    return x, y


ModelPoint = namedtuple('ModelPoint', 'x y z')