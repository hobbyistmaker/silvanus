import logging
from collections import defaultdict

import adsk.core
import adsk.fusion

from .entities import Axis
from .entities import AxisFlag
from .entities import ConfigItem
from .entities import FaceItem
from .entities import FingerType
from .entities import InputProperty
from .entities import Inputs
from .entities import PanelBody
from .entities import PanelName
from .entities import PlaneFlag

# noinspection SpellCheckingInspection
logger = logging.getLogger('silvanus.lib.generatebox.config')


# The label that will be applied for each control that will be used in the Command Input dialog

def initialize():
    # in millimeters
    _METRIC_LENGTH_DEFAULT = 285
    _METRIC_WIDTH_DEFAULT = 142.5
    _METRIC_HEIGHT_DEFAULT = 40
    _METRIC_THICKNESS_DEFAULT = 3.2
    _METRIC_KERF_DEFAULT = 0

    # in inches
    _IMPERIAL_LENGTH_DEFAULT = 12
    _IMPERIAL_WIDTH_DEFAULT = 6
    _IMPERIAL_HEIGHT_DEFAULT = 3.5
    _IMPERIAL_THICKNESS_DEFAULT = .125
    _IMPERIAL_KERF_DEFAULT = 0

    # Unit Type, Minimum, Maximum, Step - to align with the order of Fusion360 parameters.
    _METRIC_DIMENSION_DEFAULTS = ('mm', 0.5, 2500, 0.1)
    _IMPERIAL_DIMENSION_DEFAULTS = ('in', 0.03125, 48, 0.0625)

    _METRIC_THICKNESS_DEFAULTS = ('mm', 0.5, 101.6, 0.1)
    _IMPERIAL_THICKNESS_DEFAULTS = ('in', 0.03125, 4, 0.0625)

    _METRIC_KERF_DEFAULTS = ('mm', 0, 25.4, 0.01)
    _IMPERIAL_KERF_DEFAULTS = ('in', 0, 1, 0.005)

    # Combine defaults with initial values defined above
    # noinspection DuplicatedCode
    _METRIC_DEFAULTS = {
        Inputs.Length:      (*_METRIC_DIMENSION_DEFAULTS, _METRIC_LENGTH_DEFAULT),
        Inputs.Width:       (*_METRIC_DIMENSION_DEFAULTS, _METRIC_WIDTH_DEFAULT),
        Inputs.Height:      (*_METRIC_DIMENSION_DEFAULTS, _METRIC_HEIGHT_DEFAULT),
        Inputs.Thickness:   (*_METRIC_THICKNESS_DEFAULTS, _METRIC_THICKNESS_DEFAULT),
        Inputs.Kerf:        (*_METRIC_KERF_DEFAULTS, _METRIC_KERF_DEFAULT),
        Inputs.FingerWidth: (*_METRIC_THICKNESS_DEFAULTS, _METRIC_THICKNESS_DEFAULT * 3)
    }

    # noinspection DuplicatedCode
    _IMPERIAL_DEFAULTS = {
        Inputs.Length:      (*_IMPERIAL_DIMENSION_DEFAULTS, _IMPERIAL_LENGTH_DEFAULT),
        Inputs.Width:       (*_IMPERIAL_DIMENSION_DEFAULTS, _IMPERIAL_WIDTH_DEFAULT),
        Inputs.Height:      (*_IMPERIAL_DIMENSION_DEFAULTS, _IMPERIAL_HEIGHT_DEFAULT),
        Inputs.Thickness:   (*_IMPERIAL_THICKNESS_DEFAULTS, _IMPERIAL_THICKNESS_DEFAULT),
        Inputs.Kerf:        (*_IMPERIAL_KERF_DEFAULTS, _IMPERIAL_KERF_DEFAULT),
        Inputs.FingerWidth: (*_IMPERIAL_THICKNESS_DEFAULTS, _IMPERIAL_THICKNESS_DEFAULT * 3)
    }

    _INPUT_DEFAULTS = {
        Inputs.Length:      [Inputs.Length],
        Inputs.Width:       [Inputs.Width],
        Inputs.Height:      [Inputs.Height],
        Inputs.Thickness:   [
            Inputs.Thickness, Inputs.TopThickness, Inputs.BottomThickness,
            Inputs.LeftThickness, Inputs.RightThickness, Inputs.FrontThickness, Inputs.BackThickness
        ],
        Inputs.Kerf:        [Inputs.Kerf],
        Inputs.FingerWidth: [Inputs.FingerWidth]
    }

    _INPUT_ID_MAP = {
        Inputs.DimensionGroup:  'DimensionGroupId',
        Inputs.Thickness:       'thicknessDimensionInput',
        Inputs.Length:          'lengthDimensionInput',
        Inputs.Width:           'widthDimensionInput',
        Inputs.Height:          'heightDimensionInput',
        Inputs.Kerf:            'kerfDimensionInput',
        Inputs.FingerWidth:     'fingerWidthDimensionInput',
        Inputs.DimensionsTable: 'dimensionsTableInput',
        Inputs.KerfLength:      'kerfLengthCommandInput',
        Inputs.KerfWidth:       'kerfWidthCommandInput',
        Inputs.KerfHeight:      'kerfHeightCommandInput',
        Inputs.TopOffset:       'topOffsetDimensionInput',
        Inputs.BottomOffset:    'bottomOffsetDimensionInput',
        Inputs.FrontOffset:     'frontOffsetDimensionInput',
        Inputs.BackOffset:      'backOffsetDimensionInput',
        Inputs.LeftOffset:      'leftOffsetDimensionInput',
        Inputs.RightOffset:     'rightOffsetDimensionInput',
        Inputs.TopThickness:    'topThicknessDimensionInput',
        Inputs.BottomThickness: 'bottomThicknessDimensionInput',
        Inputs.FrontThickness:  'frontThicknessDimensionInput',
        Inputs.BackThickness:   'backThicknessDimensionInput',
        Inputs.LeftThickness:   'leftThicknessDimensionInput',
        Inputs.RightThickness:  'rightThicknessDimensionInput',
        Inputs.TopEnabled:      'topEnabledInput',
        Inputs.BottomEnabled:   'bottomEnabledInput',
        Inputs.FrontEnabled:    'frontEnabledInput',
        Inputs.BackEnabled:     'backEnabledInput',
        Inputs.LeftEnabled:     'leftEnabledInput',
        Inputs.RightEnabled:    'rightEnabledInput',
        Inputs.TopOverride:     'topThicknessPanelOverrideInput',
        Inputs.BottomOverride:  'bottomThicknessPanelOverrideInput',
        Inputs.FrontOverride:   'frontThicknessPanelOverrideInput',
        Inputs.BackOverride:    'backThicknessPanelOverrideInput',
        Inputs.LeftOverride:    'leftThicknessPanelOverrideInput',
        Inputs.RightOverride:   'rightThicknessPanelOverrideInput',
        Inputs.TopLabel:        'topLabelTextInput',
        Inputs.BottomLabel:     'bottomLabelTextInput',
        Inputs.FrontLabel:      'frontLabelTextInput',
        Inputs.BackLabel:       'backLabelTextInput',
        Inputs.LeftLabel:       'leftLabelTextInput',
        Inputs.RightLabel:      'rightLabelTextInput',
        Inputs.PanelTitle:      'panelTitleTextInput',
        Inputs.EnabledTitle:    'enabledTitleTextInput',
        Inputs.OverrideTitle:   'overrideTitleTextInput',
        Inputs.ThicknessTitle:  'thicknessTitleTextInput'
    }

    # Map labels to inputs
    _INPUT_LABEL_MAP = {
        'Dimensions':      [Inputs.DimensionGroup],
        'Thickness':       [Inputs.Thickness, Inputs.ThicknessTitle],
        'Length':          [Inputs.Length],
        'Width':           [Inputs.Width],
        'Height':          [Inputs.Height],
        'Kerf':            [Inputs.Kerf],
        'Finger Width':    [Inputs.FingerWidth],
        'Dimension Table': [Inputs.DimensionsTable],
        'Top':             [Inputs.TopThickness, Inputs.TopEnabled, Inputs.TopOverride, Inputs.TopLabel],
        'Bottom':          [Inputs.BottomThickness, Inputs.BottomEnabled, Inputs.BottomOverride, Inputs.BottomLabel],
        'Front':           [Inputs.FrontThickness, Inputs.FrontEnabled, Inputs.FrontOverride, Inputs.FrontLabel],
        'Back':            [Inputs.BackThickness, Inputs.BackEnabled, Inputs.BackOverride, Inputs.BackLabel],
        'Left':            [Inputs.LeftThickness, Inputs.LeftEnabled, Inputs.LeftOverride, Inputs.LeftLabel],
        'Right':           [Inputs.RightThickness, Inputs.RightEnabled, Inputs.RightOverride, Inputs.RightLabel],
        'Panel':           [Inputs.PanelTitle],
        'Enabled':         [Inputs.EnabledTitle],
        'Override':        [Inputs.OverrideTitle],
    }

    # Map inputs to labels
    _INPUT_LABELS = { value: key for key, inputs in _INPUT_LABEL_MAP.items() for value in inputs }

    ID_LABELS = list(_INPUT_LABELS.keys() & _INPUT_ID_MAP.keys())

    # Define some reused tooltip strings
    _THICKNESS_TOOLTIP = 'Override the default material thickness for the {} panel.'
    _ENABLE_TOOLTIP = 'Enable or disable the {} panel.'
    _OVERRIDE_TOOLTIP = 'Enable override of the default material thickness.'

    # The tooltip that will be displayed for inputs in the Command Input dialog
    _TOOLTIPS = {
        Inputs.Thickness:       'Default material thickness.',
        Inputs.Length:          'Outer length of the box.',
        Inputs.Width:           'Outer width of the box.',
        Inputs.Height:          'Outer height of the box.',
        Inputs.Kerf:            'Kerf adjustment to apply to the box dimensions.',
        Inputs.FingerWidth:     'Nominal width of jointed fingers.',
        Inputs.TopThickness:    _THICKNESS_TOOLTIP.format('top'),
        Inputs.BottomThickness: _THICKNESS_TOOLTIP.format('bottom'),
        Inputs.FrontThickness:  _THICKNESS_TOOLTIP.format('front'),
        Inputs.BackThickness:   _THICKNESS_TOOLTIP.format('back'),
        Inputs.LeftThickness:   _THICKNESS_TOOLTIP.format('left'),
        Inputs.RightThickness:  _THICKNESS_TOOLTIP.format('right'),
        Inputs.TopEnabled:      _ENABLE_TOOLTIP.format('top'),
        Inputs.BottomEnabled:   _ENABLE_TOOLTIP.format('bottom'),
        Inputs.FrontEnabled:    _ENABLE_TOOLTIP.format('front'),
        Inputs.BackEnabled:     _ENABLE_TOOLTIP.format('back'),
        Inputs.LeftEnabled:     _ENABLE_TOOLTIP.format('left'),
        Inputs.RightEnabled:    _ENABLE_TOOLTIP.format('right'),
        Inputs.TopOverride:     _OVERRIDE_TOOLTIP.format('top'),
        Inputs.BottomOverride:  _OVERRIDE_TOOLTIP.format('bottom'),
        Inputs.FrontOverride:   _OVERRIDE_TOOLTIP.format('front'),
        Inputs.BackOverride:    _OVERRIDE_TOOLTIP.format('back'),
        Inputs.LeftOverride:    _OVERRIDE_TOOLTIP.format('left'),
        Inputs.RightOverride:   _OVERRIDE_TOOLTIP.format('right'),
    }

    # The Fusion360 parameter name that will be used to save the value from the Command Input dialog; if needed.
    _SAVE_PARAMETERS = {
        Inputs.Length:          'length',
        Inputs.Width:           'width',
        Inputs.Height:          'height',
        Inputs.Kerf:            'kerf',
        Inputs.FingerWidth:     'finger_width',
        Inputs.Thickness:       'thickness',
        Inputs.TopThickness:    'top_thickness',
        Inputs.BottomThickness: 'bottom_thickness',
        Inputs.LeftThickness:   'left_thickness',
        Inputs.RightThickness:  'right_thickness',
        Inputs.FrontThickness:  'front_thickness',
        Inputs.BackThickness:   'back_thickness'
    }

    # Determine which checkboxes are unchecked by default -- checkboxes are True by default in the code
    _UNCHECKED_CHECKBOXES = [
        Inputs.TopEnabled
    ]

    # Define all checkboxes used by the plugin
    _CHECKBOXES = [
        Inputs.TopEnabled, Inputs.BottomEnabled, Inputs.LeftEnabled,
        Inputs.RightEnabled, Inputs.FrontEnabled, Inputs.BackEnabled
    ]

    # Determine which controls are disabled by default
    _DISABLED_CONTROLS = [
        Inputs.TopOverride, Inputs.TopThickness,
        Inputs.BottomThickness, Inputs.LeftThickness, Inputs.RightThickness,
        Inputs.FrontThickness, Inputs.BackThickness
    ]

    # Map thickness controls to associated axes
    _AXIS_CONTROLS = {
        AxisFlag.Length: [Inputs.LeftThickness, Inputs.RightThickness],
        AxisFlag.Width:  [Inputs.FrontThickness, Inputs.BackThickness],
        AxisFlag.Height: [Inputs.BottomThickness, Inputs.TopThickness]
    }
    # Reverse map axes to associated controls
    _CONTROLS_TO_AXES = { value: key for key, values in _AXIS_CONTROLS.items() for value in values }

    # Map default configuration items to the 3 main axes by which direction their large faces are oriented along
    # noinspection DuplicatedCode
    _AXES_CONFIGS = {
        AxisFlag.Height: [
            (ConfigItem.Axis, Axis(AxisFlag.Height)),
            (ConfigItem.Length, Inputs.Length),
            (ConfigItem.Width, Inputs.Width),
            (ConfigItem.Kerf, Inputs.Kerf),
            (ConfigItem.Profile, (Inputs.Length, Inputs.Width)),
            (ConfigItem.FingerWidth, Inputs.FingerWidth)
        ],
        AxisFlag.Length: [
            (ConfigItem.Axis, Axis(AxisFlag.Length)),
            (ConfigItem.Width, Inputs.Width),
            (ConfigItem.Height, Inputs.Height),
            (ConfigItem.Kerf, Inputs.Kerf),
            (ConfigItem.Profile, (Inputs.Width, Inputs.Height)),
            (ConfigItem.FingerWidth, Inputs.FingerWidth)
        ],
        AxisFlag.Width:  [
            (ConfigItem.Axis, Axis(AxisFlag.Width)),
            (ConfigItem.Length, Inputs.Length),
            (ConfigItem.Height, Inputs.Height),
            (ConfigItem.Kerf, Inputs.Kerf),
            (ConfigItem.Profile, (Inputs.Length, Inputs.Height)),
            (ConfigItem.FingerWidth, Inputs.FingerWidth)
        ]
    }

    # Map axes to associated outside panels - Because lists are ordered, this determines the order in which panels
    # are built, and can break assumptions made in the configuration that define which direction sketches and panels
    # are extruded, if changed. Panels closest to the profile sketches are drawn and extruded first
    # (Bottom, Left and Front).
    #
    # This is a result of using the first extruded body in a group as the group reference, so that we don't
    # have to redraw and extrude finger joints in every individual panel; the first panel extruded in a group
    # is used (after any finger joints are added) as the reference for the rest of the panels in that group.
    _AXIS_TO_PANEL_MAP = {
        AxisFlag.Height: [Inputs.BottomLabel, Inputs.TopLabel],
        AxisFlag.Length: [Inputs.LeftLabel, Inputs.RightLabel],
        AxisFlag.Width:  [Inputs.FrontLabel, Inputs.BackLabel]
    }

    # Map panels to axes with default configurations
    _PANEL_CONFIGS = {
        panel: {
            panel_param: param_val for panel_param, param_val in _AXES_CONFIGS[axis]
        }
        for axis, panels in _AXIS_TO_PANEL_MAP.items()
        for panel in panels
    }

    # noinspection DuplicatedCode
    _HEIGHT_PANEL_CONFIG = {
        # Face Id: ( Face Length (X), Face Width (Y), Depth)
        FaceItem.Front: {
            ConfigItem.Axis:       AxisFlag.Width,
            ConfigItem.Length:     ConfigItem.Length,
            ConfigItem.Width:      ConfigItem.Thickness,
            ConfigItem.Depth:      Inputs.FrontThickness,
            ConfigItem.Enabled:    Inputs.FrontEnabled,
            ConfigItem.FingerType: FingerType.Normal,
            ConfigItem.Joint:      Inputs.FrontLabel,
            ConfigItem.FingerAxis: (AxisFlag.Length, AxisFlag.Width)
        },
        FaceItem.Back:  {
            ConfigItem.Axis:       AxisFlag.Width,
            ConfigItem.Length:     ConfigItem.Length,
            ConfigItem.Width:      ConfigItem.Thickness,
            ConfigItem.Depth:      Inputs.BackThickness,
            ConfigItem.Enabled:    Inputs.BackEnabled,
            ConfigItem.FingerType: FingerType.Normal,
            ConfigItem.Joint:      Inputs.BackLabel,
            ConfigItem.FingerAxis: (AxisFlag.Length, AxisFlag.Width)
        },
        FaceItem.Left:  {
            ConfigItem.Axis:       AxisFlag.Length,
            ConfigItem.Length:     ConfigItem.Width,
            ConfigItem.Width:      ConfigItem.Thickness,
            ConfigItem.Depth:      Inputs.LeftThickness,
            ConfigItem.Enabled:    Inputs.LeftEnabled,
            ConfigItem.FingerType: FingerType.Normal,
            ConfigItem.Joint:      Inputs.LeftLabel,
            ConfigItem.FingerAxis: (AxisFlag.Width, AxisFlag.Length)
        },
        FaceItem.Right: {
            ConfigItem.Axis:       AxisFlag.Length,
            ConfigItem.Length:     ConfigItem.Width,
            ConfigItem.Width:      ConfigItem.Thickness,
            ConfigItem.Depth:      Inputs.RightThickness,
            ConfigItem.Enabled:    Inputs.RightEnabled,
            ConfigItem.FingerType: FingerType.Normal,
            ConfigItem.Joint:      Inputs.RightLabel,
            ConfigItem.FingerAxis: (AxisFlag.Width, AxisFlag.Length)
        }
    }
    # noinspection DuplicatedCode
    _WIDTH_PANEL_CONFIG = {
        FaceItem.Top:    {
            ConfigItem.Axis:       AxisFlag.Height,
            ConfigItem.Length:     ConfigItem.Length,
            ConfigItem.Width:      ConfigItem.Thickness,
            ConfigItem.Depth:      Inputs.TopThickness,
            ConfigItem.Enabled:    Inputs.TopEnabled,
            ConfigItem.FingerType: FingerType.Inverse,
            ConfigItem.Joint:      Inputs.TopLabel,
            ConfigItem.FingerAxis: (AxisFlag.Length, AxisFlag.Height)

        },
        FaceItem.Bottom: {
            ConfigItem.Axis:       AxisFlag.Height,
            ConfigItem.Length:     ConfigItem.Length,
            ConfigItem.Width:      ConfigItem.Thickness,
            ConfigItem.Depth:      Inputs.BottomThickness,
            ConfigItem.Enabled:    Inputs.BottomEnabled,
            ConfigItem.FingerType: FingerType.Inverse,
            ConfigItem.Joint:      Inputs.BottomLabel,
            ConfigItem.FingerAxis: (AxisFlag.Length, AxisFlag.Height)

        },
        FaceItem.Left:   {
            ConfigItem.Axis:       AxisFlag.Length,
            ConfigItem.Length:     ConfigItem.Height,
            ConfigItem.Width:      ConfigItem.Thickness,
            ConfigItem.Depth:      Inputs.LeftThickness,
            ConfigItem.Enabled:    Inputs.LeftEnabled,
            ConfigItem.FingerType: FingerType.Inverse,
            ConfigItem.Joint:      Inputs.LeftLabel,
            ConfigItem.FingerAxis: (AxisFlag.Height, AxisFlag.Length)
        },
        FaceItem.Right:  {
            ConfigItem.Axis:       AxisFlag.Length,
            ConfigItem.Length:     ConfigItem.Height,
            ConfigItem.Width:      ConfigItem.Thickness,
            ConfigItem.Depth:      Inputs.RightThickness,
            ConfigItem.Enabled:    Inputs.RightEnabled,
            ConfigItem.FingerType: FingerType.Inverse,
            ConfigItem.Joint:      Inputs.RightLabel,
            ConfigItem.FingerAxis: (AxisFlag.Height, AxisFlag.Length)
        }
    }
    # noinspection DuplicatedCode
    _LENGTH_PANEL_CONFIG = {
        FaceItem.Top:    {
            ConfigItem.Axis:       AxisFlag.Height,
            ConfigItem.Length:     ConfigItem.Width,
            ConfigItem.Width:      ConfigItem.Thickness,
            ConfigItem.Depth:      Inputs.TopThickness,
            ConfigItem.Enabled:    Inputs.TopEnabled,
            ConfigItem.FingerType: FingerType.Inverse,
            ConfigItem.Joint:      Inputs.TopLabel,
            ConfigItem.FingerAxis: (AxisFlag.Width, AxisFlag.Height)
        },
        FaceItem.Bottom: {
            ConfigItem.Axis:       AxisFlag.Height,
            ConfigItem.Length:     ConfigItem.Width,
            ConfigItem.Width:      ConfigItem.Thickness,
            ConfigItem.Depth:      Inputs.BottomThickness,
            ConfigItem.Enabled:    Inputs.BottomEnabled,
            ConfigItem.FingerType: FingerType.Inverse,
            ConfigItem.Joint:      Inputs.BottomLabel,
            ConfigItem.FingerAxis: (AxisFlag.Width, AxisFlag.Height)
        },
        FaceItem.Front:  {
            ConfigItem.Axis:       AxisFlag.Width,
            ConfigItem.Length:     ConfigItem.Height,
            ConfigItem.Width:      ConfigItem.Thickness,
            ConfigItem.Depth:      Inputs.FrontThickness,
            ConfigItem.Enabled:    Inputs.FrontEnabled,
            ConfigItem.FingerType: FingerType.Normal,
            ConfigItem.Joint:      Inputs.FrontLabel,
            ConfigItem.FingerAxis: (AxisFlag.Height, AxisFlag.Width)
        },
        FaceItem.Back:   {
            ConfigItem.Axis:       AxisFlag.Width,
            ConfigItem.Length:     ConfigItem.Height,
            ConfigItem.Width:      ConfigItem.Thickness,
            ConfigItem.Depth:      Inputs.BackThickness,
            ConfigItem.Enabled:    Inputs.BackEnabled,
            ConfigItem.FingerType: FingerType.Normal,
            ConfigItem.Joint:      Inputs.BackLabel,
            ConfigItem.FingerAxis: (AxisFlag.Height, AxisFlag.Width)
        }
    }

    yup = adsk.core.DefaultModelingOrientations.YUpModelingOrientation
    zup = adsk.core.DefaultModelingOrientations.ZUpModelingOrientation

    # noinspection DuplicatedCode
    _FACE_SELECTOR = {
        yup: {
            FaceItem.Top:    lambda body: sorted(body.faces, key=lambda f: f.centroid.y, reverse=True)[0],
            FaceItem.Bottom: lambda body: sorted(body.faces, key=lambda f: f.centroid.y)[0],
            FaceItem.Front:  lambda body: sorted(body.faces, key=lambda f: f.centroid.z, reverse=True)[0],
            FaceItem.Back:   lambda body: sorted(body.faces, key=lambda f: f.centroid.z)[0],
            FaceItem.Left:   lambda body: sorted(body.faces, key=lambda f: f.centroid.x)[0],
            FaceItem.Right:  lambda body: sorted(body.faces, key=lambda f: f.centroid.x, reverse=True)[0]
        },
        zup: {
            FaceItem.Top:    lambda body: sorted(body.faces, key=lambda f: f.centroid.z, reverse=True)[0],
            FaceItem.Bottom: lambda body: sorted(body.faces, key=lambda f: f.centroid.z)[0],
            FaceItem.Front:  lambda body: sorted(body.faces, key=lambda f: f.centroid.y)[0],
            FaceItem.Back:   lambda body: sorted(body.faces, key=lambda f: f.centroid.y, reverse=True)[0],
            FaceItem.Left:   lambda body: sorted(body.faces, key=lambda f: f.centroid.x)[0],
            FaceItem.Right:  lambda body: sorted(body.faces, key=lambda f: f.centroid.x, reverse=True)[0]
        }
    }

    # Add per-panel configuration items
    # noinspection DuplicatedCode
    _PANEL_CONFIGS[Inputs.TopLabel].update({
        ConfigItem.Name:          PanelName(_INPUT_LABELS[Inputs.TopLabel]),
        ConfigItem.Label:         Inputs.TopLabel,
        ConfigItem.Height:        Inputs.TopThickness,
        ConfigItem.Thickness:     Inputs.TopThickness,
        ConfigItem.Offset:        {
            yup: Inputs.Height,
            zup: Inputs.Height
        },
        ConfigItem.Enabled:       Inputs.TopEnabled,
        ConfigItem.Override:      Inputs.TopOverride,
        ConfigItem.Faces:         _HEIGHT_PANEL_CONFIG,
        ConfigItem.FaceSelectors: _FACE_SELECTOR
    })
    # noinspection DuplicatedCode
    _PANEL_CONFIGS[Inputs.BottomLabel].update({
        ConfigItem.Name:          PanelName(_INPUT_LABELS[Inputs.BottomLabel]),
        ConfigItem.Label:         Inputs.BottomLabel,
        ConfigItem.Height:        Inputs.BottomThickness,
        ConfigItem.Thickness:     Inputs.BottomThickness,
        ConfigItem.Offset:        {
            yup: Inputs.BottomThickness,
            zup: Inputs.BottomThickness
        },
        ConfigItem.Enabled:       Inputs.BottomEnabled,
        ConfigItem.Override:      Inputs.BottomOverride,
        ConfigItem.Faces:         _HEIGHT_PANEL_CONFIG,
        ConfigItem.FaceSelectors: _FACE_SELECTOR
    })
    # noinspection DuplicatedCode
    _PANEL_CONFIGS[Inputs.LeftLabel].update({
        ConfigItem.Name:          PanelName(_INPUT_LABELS[Inputs.LeftLabel]),
        ConfigItem.Label:         Inputs.LeftLabel,
        ConfigItem.Length:        Inputs.LeftThickness,
        ConfigItem.Thickness:     Inputs.LeftThickness,
        ConfigItem.Offset:        {
            yup: Inputs.LeftThickness,
            zup: Inputs.LeftThickness
        },
        ConfigItem.Enabled:       Inputs.LeftEnabled,
        ConfigItem.Override:      Inputs.LeftOverride,
        ConfigItem.Faces:         _LENGTH_PANEL_CONFIG,
        ConfigItem.FaceSelectors: _FACE_SELECTOR
    })
    # noinspection DuplicatedCode
    _PANEL_CONFIGS[Inputs.RightLabel].update({
        ConfigItem.Name:          PanelName(_INPUT_LABELS[Inputs.RightLabel]),
        ConfigItem.Label:         Inputs.RightLabel,
        ConfigItem.Length:        Inputs.RightThickness,
        ConfigItem.Thickness:     Inputs.RightThickness,
        ConfigItem.Offset:        {
            yup: Inputs.Length,
            zup: Inputs.Length
        },
        ConfigItem.Enabled:       Inputs.RightEnabled,
        ConfigItem.Override:      Inputs.RightOverride,
        ConfigItem.Faces:         _LENGTH_PANEL_CONFIG,
        ConfigItem.FaceSelectors: _FACE_SELECTOR
    })
    # noinspection DuplicatedCode
    _PANEL_CONFIGS[Inputs.FrontLabel].update({
        ConfigItem.Name:          PanelName(_INPUT_LABELS[Inputs.FrontLabel]),
        ConfigItem.Label:         Inputs.FrontLabel,
        ConfigItem.Width:         Inputs.FrontThickness,
        ConfigItem.Thickness:     Inputs.FrontThickness,
        ConfigItem.Offset:        {
            zup: Inputs.FrontThickness,
            yup: Inputs.Width
        },
        ConfigItem.Enabled:       Inputs.FrontEnabled,
        ConfigItem.Override:      Inputs.FrontOverride,
        ConfigItem.Faces:         _WIDTH_PANEL_CONFIG,
        ConfigItem.FaceSelectors: _FACE_SELECTOR
    })
    # noinspection DuplicatedCode
    _PANEL_CONFIGS[Inputs.BackLabel].update({
        ConfigItem.Name:          PanelName(_INPUT_LABELS[Inputs.BackLabel]),
        ConfigItem.Label:         Inputs.BackLabel,
        ConfigItem.Width:         Inputs.BackThickness,
        ConfigItem.Thickness:     Inputs.BackThickness,
        ConfigItem.Offset:        {
            zup: Inputs.Width,
            yup: Inputs.BackThickness
        },
        ConfigItem.Enabled:       Inputs.BackEnabled,
        ConfigItem.Override:      Inputs.BackOverride,
        ConfigItem.Faces:         _WIDTH_PANEL_CONFIG,
        ConfigItem.FaceSelectors: _FACE_SELECTOR
    })

    _PLANE_TO_AXIS_MAP = {
        PlaneFlag.XY: { yup: AxisFlag.Width, zup: AxisFlag.Height },
        PlaneFlag.XZ: { yup: AxisFlag.Height, zup: AxisFlag.Width },
        PlaneFlag.YZ: { yup: AxisFlag.Length, zup: AxisFlag.Length }
    }

    _PLANE_TO_OBJECT_MAP = {
        PlaneFlag.XY: lambda root: root.xYConstructionPlane,
        PlaneFlag.XZ: lambda root: root.xZConstructionPlane,
        PlaneFlag.YZ: lambda root: root.yZConstructionPlane
    }

    _DIRECTION_TO_PLANE_MAP = {
        (
            adsk.fusion.ExtentDirections.PositiveExtentDirection, 1,
            adsk.fusion.ExtentDirections.PositiveExtentDirection, -1
        ): {
            yup: [AxisFlag.Length, AxisFlag.Height, AxisFlag.Width],
            zup: [AxisFlag.Length, AxisFlag.Width, AxisFlag.Height]
        }
    }

    _PLANE_TO_DIRECTION_MAP = defaultdict(dict)

    for direction, orientations in _DIRECTION_TO_PLANE_MAP.items():
        for orientation, axes in orientations.items():
            for axis in axes:
                _PLANE_TO_DIRECTION_MAP[axis][orientation] = direction

    _PROFILE_TRANSFORM = {
        AxisFlag.Height: {
            yup: lambda x, y, z: (x, -y, z),
            zup: lambda x, y, z: (x, y, z),
        },
        AxisFlag.Width:  {
            yup: lambda x, y, z: (x, y, z),
            zup: lambda x, y, z: (x, -y, z),
        },
        AxisFlag.Length: {
            yup: lambda x, y, z: (-x, y, z),
            zup: lambda x, y, z: (-y, x, z),
        }
    }

    _PLANE_CONFIG = defaultdict(dict)

    for plane_flag, axes in _PLANE_TO_AXIS_MAP.items():
        for orientation, axis in axes.items():
            _PLANE_CONFIG[axis][orientation] = {
                ConfigItem.Plane:            plane_flag,
                ConfigItem.PlaneSelector:    _PLANE_TO_OBJECT_MAP[plane_flag],
                ConfigItem.ProfileTransform: _PROFILE_TRANSFORM[axis][orientation],
                ConfigItem.ExtentDirection:  _PLANE_TO_DIRECTION_MAP[axis][orientation]
            }

    logger.debug(f'PLANE CONFIG: {_PLANE_CONFIG}')

    _COMMAND_OVERRIDES = {
        Inputs.TopThickness:    Inputs.TopOverride,
        Inputs.BottomThickness: Inputs.BottomOverride,
        Inputs.LeftThickness:   Inputs.LeftOverride,
        Inputs.RightThickness:  Inputs.RightOverride,
        Inputs.FrontThickness:  Inputs.FrontOverride,
        Inputs.BackThickness:   Inputs.BackOverride
    }

    _THICKNESS_ENABLED = {
        Inputs.TopThickness:    Inputs.TopEnabled,
        Inputs.BottomThickness: Inputs.BottomEnabled,
        Inputs.LeftThickness:   Inputs.LeftEnabled,
        Inputs.RightThickness:  Inputs.RightEnabled,
        Inputs.FrontThickness:  Inputs.FrontEnabled,
        Inputs.BackThickness:   Inputs.BackEnabled
    }

    # Setup the main configuration map used by the plugin
    _CONFIG_MAP = {
        ConfigItem.Inputs:          {
            key: {
                InputProperty.Id:      (_INPUT_ID_MAP[key], _INPUT_LABELS[key]),
                InputProperty.Enabled: True
            } for key in ID_LABELS
        },
        ConfigItem.Tooltips:        { },
        ConfigItem.Parameters:      { },
        ConfigItem.Overrides:       _COMMAND_OVERRIDES,
        ConfigItem.Enabled:         _THICKNESS_ENABLED,
        ConfigItem.DimensionsGroup: [
            Inputs.Length, Inputs.Width, Inputs.Height, Inputs.Thickness, Inputs.FingerWidth, Inputs.Kerf
        ],
        ConfigItem.Axis:            {
            (Inputs.Length, AxisFlag.Length),
            (Inputs.Width, AxisFlag.Width),
            (Inputs.Height, AxisFlag.Height)
        },
        ConfigItem.Panels:          _PANEL_CONFIGS,
        ConfigItem.Planes:          _PLANE_CONFIG,
        ConfigItem.InputGroups:     {
            PanelBody.Top:    (Inputs.TopEnabled, Inputs.TopOverride, Inputs.TopThickness),
            PanelBody.Bottom: (Inputs.BottomEnabled, Inputs.BottomOverride, Inputs.BottomThickness),
            PanelBody.Left:   (Inputs.LeftEnabled, Inputs.LeftOverride, Inputs.LeftThickness),
            PanelBody.Right:  (Inputs.RightEnabled, Inputs.RightOverride, Inputs.RightThickness),
            PanelBody.Front:  (Inputs.FrontEnabled, Inputs.FrontOverride, Inputs.FrontThickness),
            PanelBody.Back:   (Inputs.BackEnabled, Inputs.BackOverride, Inputs.BackThickness)
        }
    }

    logger.debug(f'Config Map: {_CONFIG_MAP}')

    for key in list(_INPUT_DEFAULTS.keys() & ID_LABELS):
        for input_ in _INPUT_DEFAULTS[key]:
            _CONFIG_MAP[ConfigItem.Inputs][input_][InputProperty.Defaults] = {
                True:  _METRIC_DEFAULTS[key],
                False: _IMPERIAL_DEFAULTS[key]
            }

    for key in list(_CONTROLS_TO_AXES.keys() & ID_LABELS):
        _CONFIG_MAP[ConfigItem.Inputs][key][InputProperty.Axis] = _CONTROLS_TO_AXES[key]

    _CONFIG_MAP[ConfigItem.Inputs][Inputs.DimensionsTable].update({
        InputProperty.Defaults: (0, '4:1:2:1:1:2:1:5'),
        InputProperty.Titles:   [
            (Inputs.PanelTitle, 0, 0),
            (Inputs.EnabledTitle, 1, 2),
            (Inputs.OverrideTitle, 4, 2),
            (Inputs.ThicknessTitle, 7, 0)
        ],
        InputProperty.Rows:     [
            (Inputs.TopLabel, Inputs.TopEnabled, Inputs.TopOverride, Inputs.TopThickness),
            (Inputs.BottomLabel, Inputs.BottomEnabled, Inputs.BottomOverride, Inputs.BottomThickness),
            (Inputs.LeftLabel, Inputs.LeftEnabled, Inputs.LeftOverride, Inputs.LeftThickness),
            (Inputs.RightLabel, Inputs.RightEnabled, Inputs.RightOverride, Inputs.RightThickness),
            (Inputs.FrontLabel, Inputs.FrontEnabled, Inputs.FrontOverride, Inputs.FrontThickness),
            (Inputs.BackLabel, Inputs.BackEnabled, Inputs.BackOverride, Inputs.BackThickness)
        ]
    })

    for key in list(_TOOLTIPS.keys() & ID_LABELS):
        _CONFIG_MAP[ConfigItem.Tooltips][key] = _TOOLTIPS[key]

    for key in list(_SAVE_PARAMETERS.keys() & ID_LABELS):
        _CONFIG_MAP[ConfigItem.Parameters][key] = {
            ConfigItem.Name:    _SAVE_PARAMETERS[key],
            ConfigItem.Enabled: True
        }

    _CONFIG_MAP[ConfigItem.Parameters][Inputs.Thickness][ConfigItem.Enabled] = False

    for key in set(_CHECKBOXES).intersection(ID_LABELS):
        _CONFIG_MAP[ConfigItem.Inputs][key][InputProperty.TurnedOn] = True

    for key in set(_UNCHECKED_CHECKBOXES).intersection(ID_LABELS):
        _CONFIG_MAP[ConfigItem.Inputs][key][InputProperty.TurnedOn] = False

    for key in set(_DISABLED_CONTROLS).intersection(ID_LABELS):
        _CONFIG_MAP[ConfigItem.Inputs][key][InputProperty.Enabled] = False

    return _CONFIG_MAP


# Initialize the configuration dictionary on module load. This dictionary should not be changed by plugin operations.
config = initialize()
