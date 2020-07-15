from collections import defaultdict
from dataclasses import dataclass
from dataclasses import field
from enum import Enum
from typing import Any
from typing import Callable
from typing import Dict
from typing import List
from typing import NamedTuple
from typing import Tuple


class AxisFlag(Enum):
    Height = 1
    Length = 2
    Width = 3


class ConfigItem(Enum):
    Axis = 1
    Control = 11
    Depth = 2
    DimensionsGroup = 3
    Enabled = 4
    ExtentDirection = 5
    Face = 6
    FaceAxis = 7
    Faces = 8
    FaceSelectors = 9
    Fingers = 10
    FingerAxis = 21
    FingerCount = 22
    FingerCuts = 20
    FingerDepth = 23
    FingerOffset = 24
    FingerPatternDistance = 25
    FingerType = 26
    FingerWidth = 27
    GroupName = 28
    Height = 29
    InputGroups = 41
    Inputs = 42
    Joint = 43
    Joints = 44
    Kerf = 45
    Label = 46
    Length = 47
    Name = 48
    Offset = 49
    Offsets = 60
    Orientation = 61
    Override = 62
    Overrides = 59
    Panel = 63
    PanelData = 64
    Panels = 65
    Parameters = 66
    Plane = 67
    Planes = 68
    PlaneSelector = 69
    Profile = 81
    ProfileName = 82
    ProfileTransform = 83
    SimpleDividers = 84
    Thickness = 85
    ThicknessGroups = 86
    Tooltips = 87
    Width = 88


class FaceItem(Enum):
    Top = 1
    Bottom = 2
    Left = 3
    Right = 4
    Front = 5
    Back = 6


class JointItem(Enum):
    Axis = 1
    JointOffset = 2
    JointPanels = 3
    JointParent = 4
    Type = 5
    FaceDistance = 6
    ParentPanels = 7
    FingerConfig = 8
    ParentPanel = 9


class GroupItem(Enum):
    Names = 1
    Thicknesses = 2
    Profile = 3
    Transform = 4
    PlaneSelector = 5
    Offsets = 6
    Panels = 7
    Joints = 8


class InputProperty(Enum):
    Id = 0
    Label = 1
    Defaults = 2
    Titles = 3
    Rows = 4
    Enabled = 5
    TurnedOn = 6
    Axis = 7


class FingerType(Enum):
    Normal = 1
    Inverse = 2


class Inputs(Enum):
    DimensionGroup = 0
    Thickness = 1
    Length = 2
    Width = 3
    Height = 4
    Kerf = 5
    FingerWidth = 6
    DimensionsTable = 7

    KerfLength = 10
    KerfWidth = 11
    KerfHeight = 12

    TopOffset = 20
    BottomOffset = 21
    FrontOffset = 22
    BackOffset = 23
    LeftOffset = 24
    RightOffset = 25

    TopThickness = 30
    BottomThickness = 31
    FrontThickness = 32
    BackThickness = 33
    LeftThickness = 34
    RightThickness = 35

    TopEnabled = 40
    BottomEnabled = 41
    FrontEnabled = 42
    BackEnabled = 43
    LeftEnabled = 44
    RightEnabled = 45

    TopOverride = 50
    BottomOverride = 51
    FrontOverride = 52
    BackOverride = 53
    LeftOverride = 54
    RightOverride = 55

    TopLabel = 60
    BottomLabel = 61
    FrontLabel = 62
    BackLabel = 63
    LeftLabel = 64
    RightLabel = 65

    PanelTitle = 70
    EnabledTitle = 71
    OverrideTitle = 72
    ThicknessTitle = 73

    SimpleDividerGroup = 80
    LengthDivider = 81
    LengthDividerTitle = 82
    WidthDivider = 83
    WidthDividerTitle = 84
    HeightDivider = 85
    HeightDividerTitle = 86

    ErrorMessage = 255


class JointFace(Enum):
    Top = 1
    Bottom = 2
    Left = 3
    Right = 4
    Front = 5
    Back = 6
    Top_Bottom = 10
    Left_Right = 11
    Front_Back = 12


class PanelInstance(Enum):
    Top = 1
    Bottom = 2
    Left = 3
    Right = 4
    Front = 5
    Back = 6


class PanelType(Enum):
    Inside = 1
    Outside = 2


class PlaneFlag(Enum):
    XY = 1
    XZ = 2
    YZ = 3


class OutsideType(Enum):
    Top = 1
    Bottom = 2
    Left = 3
    Right = 4
    Front = 5
    Back = 6


@dataclass(eq=True, frozen=True)
class Axis:
    value: AxisFlag


@dataclass
class Component:
    value: Any


@dataclass(eq=True, frozen=True)
class DefaultFingers:
    value: float
    expression: str


@dataclass(eq=True, frozen=True)
class EstimatedFingers:
    value: float
    expression: str


@dataclass(eq=True, frozen=True)
class ActualFingerWidth:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class FingerOffset:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class ActualFingerCount:
    value: float
    expression: str


@dataclass(eq=True, frozen=True)
class FingerPatternDistance:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class Enabled:
    value: bool


@dataclass
class FingerDepth:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class FingerWidth:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class Height:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class Kerf:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class Length:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class PanelOffset:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class Orientation:
    value: int


@dataclass(eq=True, frozen=True)
class Override:
    value: bool


@dataclass(eq=True, frozen=True)
class PanelName:
    value: str


@dataclass
class PanelPlane:
    value: Any


@dataclass
class PanelsList:
    panels: List[Any]


@dataclass
class PlaneSelector:
    plane: Any


@dataclass
class ProfileTransform:
    func: Any


@dataclass(eq=True, frozen=True)
class Thickness:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class Width:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class LengthWidthProfile:
    length: Length
    width: Width


class PanelProfile(LengthWidthProfile): pass


class TopBottomFaceProfile(LengthWidthProfile): pass


class FrontBackFaceProfile(LengthWidthProfile): pass


class LeftRightFaceProfile(LengthWidthProfile): pass


class KerfJoint: pass


class PanelJoint: pass


class OutsidePanel: pass


class InsidePanel: pass


@dataclass
class Panel:
    axis: Axis
    name: str
    orientation: int
    enabled: Enabled
    finger_width: FingerWidth
    height: Height
    kerf: Kerf
    length: Length
    offset: PanelOffset
    override: Override
    profile: PanelProfile
    thickness: Thickness
    width: Width
    transform: ProfileTransform
    plane_selector: PlaneSelector
    direction: Any
    face_selectors: Dict
    faces: Dict = field(default_factory=lambda: defaultdict(lambda: defaultdict(dict)))


@dataclass(eq=True, frozen=True)
class AxisDirection:
    profile_extent_direction: int
    profile_multiplier: int
    copy_extent_direction: int
    copy_multiplier: int


@dataclass(eq=True, frozen=True)
class Parameter:
    name: str
    value: float
    unitType: str


class PanelDefinition(NamedTuple):
    panel_type: PanelType
    name: Inputs
    enable: Inputs
    override: Inputs
    thickness: Inputs
    length: Inputs
    width: Inputs
    height: Inputs
    kerf: Inputs
    finger_width: Inputs
    finger_types: Dict[FingerType, List[AxisFlag]]
    reference_point: Tuple[Inputs, Inputs, Inputs]
    max_reference: Inputs
    orientation: AxisFlag


@dataclass
class PanelDimensions:
    length: Length
    width: Width
    height: Height
    orientation: AxisFlag


@dataclass
class FaceDimensions:
    length: Length
    width: Width
    height: Height
    orientation: AxisFlag


@dataclass
class ControlInput:
    control: Any


class ThicknessInput(ControlInput): ...


class LengthInput(ControlInput): ...


class WidthInput(ControlInput): ...


class HeightInput(ControlInput): ...


class KerfInput(ControlInput): ...


class FingerWidthInput(ControlInput): ...


@dataclass
class BooleanInput:
    control: Any = field(hash=False)


class EnableInput(BooleanInput): ...


class OverrideInput(BooleanInput): ...


@dataclass
class EnableThicknessPair:
    enabled: EnableInput
    thickness: ThicknessInput


@dataclass(eq=True, frozen=True)
class PanelOrientation:
    axis: AxisFlag


class LengthOrientation: ...


class WidthOrientation: ...


class HeightOrientation: ...


@dataclass(eq=True, frozen=True)
class ParentOrientation:
    value: AxisFlag


@dataclass(eq=True, frozen=True)
class FingerPatternType:
    finger_type: FingerType


class LengthFingerPatternType(FingerPatternType): ...


class WidthFingerPatternType(FingerPatternType): ...


class HeightFingerPatternType(FingerPatternType): ...


class InverseFingerPattern: ...


class NormalFingerPattern: ...


@dataclass(eq=True, frozen=True)
class FingerOrientation:
    axis: AxisFlag


@dataclass(eq=True, frozen=True)
class ModelOrientation:
    axis: int


@dataclass(eq=True, frozen=True)
class Renderable: ...


@dataclass(eq=True, frozen=True)
class RootComponent:
    value: Any


@dataclass(eq=True, frozen=True)
class GroupOrientation:
    value: AxisFlag


@dataclass(eq=True, frozen=True)
class GroupProfile:
    length: str
    width: str


@dataclass(eq=True, frozen=True)
class FingerTypes:
    values: List[FingerPatternType]


@dataclass(eq=True, frozen=True)
class GroupName:
    value: str


@dataclass(eq=True, frozen=True)
class GroupTransform:
    call: Callable


@dataclass(eq=True, frozen=True)
class GroupPlaneSelector:
    call: Callable


@dataclass(eq=True, frozen=True)
class PanelEndReferencePoint:
    length: Length
    width: Width
    height: Height


@dataclass(eq=True, frozen=True)
class PanelStartReferencePoint:
    length: Length
    width: Width
    height: Height


@dataclass(eq=True, frozen=True)
class ReferencePointInput:
    length: Any
    width: Any
    height: Any


@dataclass(eq=True, frozen=True)
class MaxOffset:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class StartPanelOffset:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class MaxOffsetInput:
    control: Any


@dataclass(eq=True, frozen=True)
class ExtrusionDistance:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class ExtrusionOffset:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class GroupPanel:
    name: str
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class GroupPanels:
    thickness: Thickness
    panels: List[GroupPanel]


@dataclass(eq=True, frozen=True)
class PanelSubGroups:
    groups: List[GroupPanels]


@dataclass(eq=True, frozen=True)
class ParentPanel:
    id: int


@dataclass(eq=True, frozen=True)
class JointName:
    value: str


@dataclass(eq=True, frozen=True)
class JointThickness:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class JointPanelOffset:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class JointPatternDistance:
    value: float
    expression: str
    unitType: str


@dataclass(eq=True, frozen=True)
class JointProfile:
    profile: Any


class JointGroup:

    def __init__(self):
        self.names = set()
        self.joint = None
        self.joint_extrusions = set()


class JointOrientationGroup:

    def __init__(self):
        self.joint_profiles = defaultdict(lambda: JointGroup())


class JointTypeGroup:

    def __init__(self):
        self.joint_orientations = defaultdict(lambda: JointOrientationGroup())


class PanelGroup:

    def __init__(self):
        self.joint_types = defaultdict(lambda: JointTypeGroup())
        self.extrusions = set()
        self.panel = None


class ProfileGroup:

    def __init__(self):
        self.names = set()
        self.transform = None
        self.plane_selector = None
        self.panels = defaultdict(lambda: PanelGroup())


class AxisGroup:

    def __init__(self):
        self.profiles = defaultdict(lambda: ProfileGroup())
