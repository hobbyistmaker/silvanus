from typing import List

import adsk.fusion

class Profile:

    def __init__(self, sketch, parent=None, lines=None, dimensions=None):
        self.parent = parent
        self.sketch = sketch
        self.dimensions = dimensions

        sketch_lines = sketch.sketchCurves.sketchLines
        self.lines = lines if lines else self._lines(sketch_lines)
        self.sorted_lines = sorted(self.lines, key=lambda l: l.length, reverse=True)

        self.long_line = self.sorted_lines[0]
        self.long_lines = self.sorted_lines[0:2]
        self.short_lines = self.sorted_lines[2:4]

        self.is_vertical = self.parent.is_vertical if self.parent else _is_vertical(self.long_line)
        self.is_reversed = _line_is_reversed(self.long_line)

        self.left = left(self.lines)
        self.right = right(self.lines)
        self.top = top(self.lines)
        self.bottom = bottom(self.lines)

    @staticmethod
    def _lines(lines) -> List[adsk.fusion.SketchLine]:
        return lines[0:4]

    @property
    def profile(self):
        return self.inner_profile if self.parent else self.outer_profile

    @property
    def outer_profile(self):
        return self.sketch.profiles.item(0)

    @property
    def inner_profile(self):
        return self.sketch.profiles.item(1)


def mid_x(line): return (line.startSketchPoint.geometry.x + line.endSketchPoint.geometry.x) / 2
def mid_y(line): return (line.startSketchPoint.geometry.y + line.endSketchPoint.geometry.y) / 2
def max_x(lines): return sorted(lines, key=lambda line: mid_x(line), reverse=True)[0]
def max_y(lines): return sorted(lines, key=lambda line: mid_y(line), reverse=True)[0]
def max_point(points): return sorted(points, key=lambda point: point.geometry.asArray(), reverse=True)[0]
def min_x(lines): return sorted(lines, key=lambda line: mid_x(line))[0]
def min_y(lines): return sorted(lines, key=lambda line: mid_y(line))[0]
def min_point(points): return sorted(points, key=lambda point: point.geometry.asArray())[0]
def left(lines): return min_x(lines)
def right(lines): return max_x(lines)
def top(lines): return max_y(lines)
def bottom(lines): return min_y(lines)

def _is_reversed(first, last):
    fp = (first.worldGeometry.x, first.worldGeometry.y, first.worldGeometry.z)
    lp = (last.worldGeometry.x, last.worldGeometry.y, last.worldGeometry.z)

    is_vertical = first.geometry.x < 0
    return True if (lp < fp) or (is_vertical and (fp < lp)) else False


def _line_is_reversed(line):
    first, last = line.startSketchPoint, line.endSketchPoint
    return _is_reversed(first, last)


def _is_vertical(line):
    start = line.startSketchPoint.geometry
    end = line.endSketchPoint.geometry
    return True if (start.x == end.x) else False
