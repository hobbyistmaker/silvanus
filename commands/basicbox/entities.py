import math

from abc import ABC
from dataclasses import dataclass
from dataclasses import field
from typing import Dict
from typing import Tuple


@dataclass
class Parameter:

    expression: str
    value: float

    @classmethod
    def ceiling(cls, parameter):
        if not isinstance(parameter, Parameter):
            raise RuntimeError('Reference is not a valid parameter.')
        return Parameter(f'ceil({parameter.expression})', math.ceil(parameter.value))

    @classmethod
    def floor(cls, parameter):
        if not isinstance(parameter, Parameter):
            raise RuntimeError('Reference is not a valid parameter.')
        return Parameter(f'floor({parameter.expression})', math.floor(parameter.value))

    @classmethod
    def max(cls, first, second):
        first = first if isinstance(first, Parameter) else Parameter(f'{first}', first)
        second = second if isinstance(second, Parameter) else Parameter(f'{second}', second)

        if not isinstance(first, Parameter) or not isinstance(second, Parameter):
            raise RuntimeError('References are not valid parameters.')

        first = cls._check_value(first)
        second = cls._check_value(second)
        return Parameter(f'max({first.expression}; {second.expression})', max(first.value, second.value))

    def __eq__(self, other):
        if isinstance(other, Parameter):
            return other.value == self.value and other.expression == self.expression
        elif isinstance(other, str):
            return other == self.expression
        elif isinstance(other, (int, float)):
            return other == self.value
        else:
            raise NotImplemented

    def __gt__(self, other):
        if isinstance(other, Parameter):
            return self.value > other.value
        elif isinstance(other, (int, float)):
            return self.value > other
        else:
            raise NotImplemented

    def __lt__(self, other):
        if isinstance(other, Parameter):
            return self.value < other.value
        elif isinstance(other, (int, float)):
            return self.value < other
        else:
            raise NotImplemented

    def __bool__(self):
        if isinstance(self.value, (int, float)):
            return self.value > 0
        elif isinstance(self.value, str):
            return self.value is not None and self.value is not ''
        elif isinstance(self.value, bool):
            return self.value is not None and self.value is not False
        else:
            return NotImplemented

    def __round__(self):
        return Parameter(f'round({self.expression})', round(self.value, 0))

    def __float__(self):
        return float(self.value)

    def __ceil__(self):
        return Parameter(f'ceil({self.expression})', math.ceil(self.value))

    def __floor__(self):
        return Parameter(f'floor({self.expression})', math.floor(self.value))

    def __add__(self, other):
        if other is None:
            return self
        else:
            return self._operate_other(other, '+', lambda o: self.value + o)

    def __sub__(self, other):
        if other is None:
            return self
        else:
            return self._operate_other(other, '-', lambda o: self.value - o)

    def __mul__(self, other):
        if other is None:
            return self
        else:
            return self._operate_other(other, '*', lambda o: self.value * o)

    def __neg__(self):
        return Parameter(f'-({self.expression})', -self.value)

    def __iadd__(self, other):
        return self.__add__(other)

    def __isub__(self,other):
        return self.__sub__(other)

    def __radd__(self, other):
        if other is None:
            return self
        else:
            return self.__add__(other)

    def __rmul__(self, other):
        if other is None:
            return self
        else:
            return self.__mul__(other)

    def __rsub__(self, other):
        if other is None:
            return self
        elif isinstance(other, (float, int)):
            return Parameter(f'{other}', other) - self
        else:
            return NotImplemented

    def __str__(self):
        return f'{self.expression}'

    def __truediv__(self, other):
        if other is None:
            return self
        else:
            return self._operate_other(other, '/', lambda o: self.value / o)

    def __floordiv__(self, other):
        if other is None:
            return self
        else:
            return self._format_other(f'floor({self.expression} / {{}})', other, lambda o: self.value // o)

    @classmethod
    def _format_other(cls, expression, other, func=None):
        if isinstance(other, Parameter):
            result = func(other.value) if func else None
            return Parameter(expression.format(other.expression), result)
        if isinstance(other, (int, float)):
            result = func(other) if func else None
            return Parameter(expression.format(other), result)

    @classmethod
    def _check_value(cls, value):
        if isinstance(value, Parameter):
            return value
        if isinstance(value, (int, float)):
            return Parameter(f'{value}', value)

    def _operate_other(self, other, symbol, func):
        return self._format_other(f'({self.expression} {symbol} {{}})', other, func)


@dataclass
class Divider:
    quantity: Parameter
    distance: Parameter
    thickness: Parameter
    offset: Parameter


class AbstractJoint(ABC):
    pass


@dataclass
class Joint(AbstractJoint):
    name: str
    finger_width: Parameter
    fingers: Parameter
    distance: Parameter
    offset: Parameter
    inverse: bool
    kerf: Parameter = None
    depth: Parameter = field(init=False)
    sibling: AbstractJoint = field(init=False)
    inside: Parameter = field(init=False, default=False)

    def __eq__(self, other):
        if other:
            width_eq = other.finger_width == self.finger_width
            fingers_eq = other.fingers == self.fingers
            distance_eq = other.distance == self.distance
            offset_eq = other.offset == self.offset
            inverse_eq = other.inverse == self.inverse
            depth_eq = float(getattr(other, 'depth', 0)) == float(getattr(self, 'depth', 0))
            return width_eq and fingers_eq and distance_eq and offset_eq and inverse_eq and depth_eq
        return False


@dataclass
class JointSibling(AbstractJoint):
    distance: Parameter
    joint: AbstractJoint
    # quantity: Parameter


@dataclass
class JointParameters:
    length: Parameter
    finger_width: Parameter

    @property
    def default_fingers(self):
        return math.ceil(self.length / self.finger_width)

    @property
    def estimated_fingers(self):
        return Parameter.max(3, (math.floor(self.default_fingers / 2) * 2) - 1)

    @property
    def actual_width(self):
        return self.length / self.estimated_fingers

    @property
    def offset(self):
        return self.actual_width

    @property
    def actual_fingers(self):
        return math.floor(self.estimated_fingers / 2)

    @property
    def distance(self):
        return (self.estimated_fingers - 3) * self.actual_width


@dataclass
class KerfJointParameters(JointParameters):
    kerf: Parameter

    @property
    def kerf_width(self):
        return self.actual_width - self.kerf

    @property
    def kerf_offset(self):
        return self.offset + self.kerf

    @property
    def kerf_distance(self):
        return self.distance - self.kerf


@dataclass
class InverseJointParameters(JointParameters):

    @property
    def offset(self):
        return Parameter('0', 0)

    @property
    def actual_fingers(self):
        return math.ceil(self.estimated_fingers / 2)

    @property
    def distance(self):
        return (self.estimated_fingers - 1) * self.actual_width


@dataclass
class InverseKerfJointParameters(InverseJointParameters, KerfJointParameters):
    pass


@dataclass
class Face:
    name: str
    length: Parameter
    width: Parameter
    kerf: Parameter
    joint: Joint = field(init=False, default=None)


@dataclass
class Panel:
    name: str
    length: Parameter
    width: Parameter
    thickness: Parameter
    kerf: Parameter = None
    enabled: bool = True

    start: Tuple[float, float] = field(init=False)
    end: Tuple[float, float] = field(init=False)

    top: Face = field(init=False)
    bottom: Face = field(init=False)
    left: Face = field(init=False)
    right: Face = field(init=False)
    front: Face = field(init=False)
    back: Face = field(init=False)

    def __post_init__(self):
        self.start = (0, 0)
        self.end = (float(self.length), float(self.width))

    def __bool__(self):
        return getattr(self, 'enabled', False)


@dataclass
class Box:
    length: Parameter
    width: Parameter
    height: Parameter
    thickness: Parameter
    kerf: Parameter

    top: Panel = field(init=False)
    bottom: Panel = field(init=False)
    left: Panel = field(init=False)
    right: Panel = field(init=False)
    front: Panel = field(init=False)
    back: Panel = field(init=False)

    def __post_init__(self):
        self.top, self.bottom = self._create_top_and_bottom()
        self.left, self.right = self._create_left_and_right()
        self.front, self.back = self._create_front_and_back()

    def _create_top_and_bottom(self):
        def panel(name): return Panel(name, self.length, self.width, self.thickness, self.kerf)
        top = panel('Top')
        bottom = panel('Bottom')
        self._define_top_bottom_faces([top, bottom])
        return top, bottom

    def _define_top_bottom_faces(self, panels):
        for panel in panels:
            self._define_faces(panel,
                               top_bottom=(self.length, self.width, self.kerf),
                               left_right=(self.width, self.thickness, self.kerf),
                               front_back=(self.length, self.thickness, self.kerf))

    def _create_left_and_right(self):
        def panel(name): return Panel(name, self.height, self.width, self.thickness, self.kerf)
        left = panel('Left')
        right = panel('Right')
        self._define_left_right_faces([left, right])
        return left, right

    def _define_left_right_faces(self, panels):
        for panel in panels:
            self._define_faces(panel,
                               top_bottom=(self.width, self.thickness, self.kerf),
                               left_right=(self.width, self.height, self.kerf),
                               front_back=(self.height, self.thickness, self.kerf))

    def _create_front_and_back(self):
        def panel(name): return Panel(name, self.length, self.height, self.thickness, self.kerf)
        front = panel('Front')
        back = panel('Back')
        self._define_front_back_faces([front, back])
        return front, back

    def _define_front_back_faces(self, panels):
        for panel in panels:
            self._define_faces(panel,
                               top_bottom=(self.length, self.thickness, self.kerf),
                               left_right=(self.height, self.thickness, self.kerf),
                               front_back=(self.length, self.height, self.kerf))

    @staticmethod
    def _define_faces(panel, *, top_bottom, left_right, front_back):
        panel.left = Face(f'{panel.name} Left', *left_right)
        panel.right = Face(f'{panel.name} Right', *left_right)
        panel.front = Face(f'{panel.name} Front', *front_back)
        panel.back = Face(f'{panel.name} Back', *front_back)
        panel.top = Face(f'{panel.name} Top', *top_bottom)
        panel.bottom = Face(f'{panel.name} Bottom', *top_bottom)


@dataclass
class PanelGroup:
    first: Panel
    second: Panel
    dividers: int


@dataclass
class Parameters:
    parameters: Dict[str, Parameter] = field(default_factory=lambda: {})
    initialized: bool = True

    def from_(self, item):
        name = getattr(item, 'name', None)
        value = getattr(item, 'value', 0)
        expression = getattr(item, 'expression', name)
        self.add(name, value, expression)

    def add(self, name, value, expression):
        parameter = Parameter(expression, value)
        self.update(name, parameter)

    def update(self, name, parameter):
        self.parameters[name] = parameter

    def __contains__(self, item):
        return item in self.parameters

    def __getattr__(self, item):
        if item in self.parameters:
            return self.parameters[item]

    def __getitem__(self, item):
        if item in self.parameters:
            return self.parameters[item]

    def __setattr__(self, key, value):
        if 'initialized' not in self.__dict__:
            return dict.__setattr__(self, key, value)
        elif key in self.__dict__:
            dict.__setattr__(self, key, value)
        else:
            self.parameters[key] = value