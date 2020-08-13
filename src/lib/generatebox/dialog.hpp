//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/22/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//

#ifndef SILVANUSPRO_DIALOG_HPP
#define SILVANUSPRO_DIALOG_HPP

#include <map>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include <entt/entt.hpp>

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include "entities/DialogInputs.hpp"
#include "entities/AxisFlag.hpp"
#include "entities/FingerMode.hpp"
#include "entities/Position.hpp"
#include "entities/JointType.hpp"

namespace silvanus::generatebox {

    struct ParameterizedControl {
        std::string                               name;
        adsk::core::Ptr<adsk::core::CommandInput> control;
    };

    struct EnableThicknessPair {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput>    enable;
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> thickness;
    };

    struct InputDefaults {
        const std::string unit_type;
        const float       minimum;
        const float       maximum;
        const float       step;
        const float       initial_value;
    };

    struct InputConfig {
        const entities::DialogInputs lookup;
        const std::string            parameter;
        const std::string            id;
        const std::string            name;
        const std::string            unit_type;
        const float                  minimum;
        const float                  maximum;
        const float                  step;
        const float                  initial_value;
    };

    struct PanelThicknessConfig {
        const entities::DialogInputs                  lookup;
        const std::string                             parameter;
        const std::string                             id;
        const std::string                             name;
        const std::unordered_map<bool, InputDefaults> defaults;
    };

    struct EnabledInputConfig {
        const entities::DialogInputs lookup;
        const std::string            id;
        const std::string            name;
        const bool                   default_value;
    };

    struct OverrideInputConfig {
        const entities::DialogInputs lookup;
        const std::string            id;
        const std::string            name;
    };

    struct PanelLabelConfig {
        const std::string id;
        const std::string name;
    };

    struct DimensionTableTitle {
        const std::string id;
        const std::string name;
        const std::string label;
        const int         column;
        const int         span;
    };

    struct PanelReferencePointIds {
        const entities::DialogInputs length;
        const entities::DialogInputs width;
        const entities::DialogInputs height;
        const entities::DialogInputs thickness;
        const entities::DialogInputs max;
    };

    using jointTypes = std::vector<std::tuple<entities::JointType, std::vector<entities::AxisFlag>>>;
    using jointPositions = std::map<entities::Position, jointTypes>;
    using inputAxisMaps = std::vector<std::tuple<entities::DialogInputs, entities::AxisFlag>>;
    using inputConfigs = std::unordered_map<bool, std::vector<InputConfig>>;
    using axisList = std::vector<entities::AxisFlag>;

    struct DimensionTableRow {
        const PanelLabelConfig       label;
        const EnabledInputConfig     enable;
        const OverrideInputConfig    override;
        const PanelThicknessConfig   thickness;
        const entities::AxisFlag     orientation;
        const std::unordered_map<adsk::core::DefaultModelingOrientations, PanelReferencePointIds> reference;
        jointPositions            fingers;
    };

    struct DimensionTable {
        const std::string                      id;
        const std::string                      name;
        const std::string                      column_ratio;
        const std::vector<DimensionTableTitle> titles;
        const std::vector<DimensionTableRow>   dimensions;
    };

    struct EnableThicknessControl {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput>    is_enabled;
        adsk::core::Ptr<adsk::core::BoolValueCommandInput>    is_overridden;
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> use_override;
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> use_default;
    };

    class CreateDialog {

        inputAxisMaps m_axis_to_dimension = {
            std::make_tuple(entities::DialogInputs::Length, entities::AxisFlag::Length),
            std::make_tuple(entities::DialogInputs::Width, entities::AxisFlag::Width),
            std::make_tuple(entities::DialogInputs::Height, entities::AxisFlag::Height)
        };

        inputConfigs m_dimension_inputs = {
            {
                true,  {
                           InputConfig{entities::DialogInputs::Length, "length", "lengthSpinnerInput", "Length", "mm", 0.01, 2540, .1, 285},
                           InputConfig{entities::DialogInputs::Width, "width", "widthSpinnerInput", "Width", "mm", 0.01, 2540, .1, 142.5},
                           InputConfig{entities::DialogInputs::Height, "height", "heightSpinnerInput", "Height", "mm", 0.01, 2540, .1, 40},
                           InputConfig{entities::DialogInputs::Thickness, "thickness", "thicknessSpinnerInput", "Thickness", "mm", 0.01, 2540, .1, 3.2},
                           InputConfig{
                               entities::DialogInputs::FingerWidth, "finger_width", "fingerWidthSpinnerInput", "Finger Width", "mm", 0.01, 50.4,
                               .1, 9.6
                           },
                           InputConfig{entities::DialogInputs::Kerf, "kerf", "kerfSpinnerInput", "Kerf", "mm", 0, 25.4, .05, 0}
                       }},
            {
                false, {
                           InputConfig{entities::DialogInputs::Length, "length", "lengthSpinnerInput", "Length", "in", 0.005, 48, 0.0625, 12},
                           InputConfig{entities::DialogInputs::Width, "width", "widthSpinnerInput", "Width", "in", 0.005, 48, 0.0625, 6},
                           InputConfig{entities::DialogInputs::Height, "height", "heightSpinnerInput", "Height", "in", 0.005, 48, 0.0625, 3.5},
                           InputConfig{
                               entities::DialogInputs::Thickness, "thickness", "thicknessSpinnerInput", "Thickness", "in", 0.005, 48, 0.0625,
                               .125
                           },
                           InputConfig{
                               entities::DialogInputs::FingerWidth, "finger_width", "fingerWidthSpinnerInput", "Finger Width", "in", 0.005, 48,
                               0.0625, .375
                           },
                           InputConfig{entities::DialogInputs::Kerf, "kerf", "kerfSpinnerInput", "Kerf", "in", 0, 1, .05, 0}
                       }}
        };

        DimensionTable m_dimensions_table = {
            "dimensionsTableInput", "Dimensions", "4:1:2:1:1:2:1:5",
            {
                DimensionTableTitle{"panelTitleTextInput", "Panel", "Panel", 0, 0},
                DimensionTableTitle{"enableTitleTextInput", "Enable", "Enable", 1, 2},
                DimensionTableTitle{"overrideTitleTextInput", "Override", "Override", 4, 2},
                DimensionTableTitle{"thicknessTitleTextInput", "Thickness", "Thickness", 7, 0}
            },
            {
                DimensionTableRow{
                    PanelLabelConfig{"topPanelLabel", "Top"},
                    EnabledInputConfig{entities::DialogInputs::TopEnable, "topEnableInput", "Top", false},
                    OverrideInputConfig{entities::DialogInputs::TopOverride, "topOverrideInput", "Top"},
                    PanelThicknessConfig{
                        entities::DialogInputs::TopThickness, "top_thickness", "topThicknessInput", "Top",
                        std::unordered_map<bool, InputDefaults>{
                            {true,  InputDefaults{"mm", 0.01, 2540, .1, 3.2}},
                            {false, InputDefaults{"in", 0.005, 48, 0.0625, .125}}
                        }},
                    entities::AxisFlag::Height,
                    {{
                        adsk::core::YUpModelingOrientation, PanelReferencePointIds{
                            entities::DialogInputs::Length,
                            entities::DialogInputs::Width,
                            entities::DialogInputs::Height,
                            entities::DialogInputs::TopThickness,
                            entities::DialogInputs::Height
                    }},{
                        adsk::core::ZUpModelingOrientation, PanelReferencePointIds{
                            entities::DialogInputs::Length,
                            entities::DialogInputs::Width,
                            entities::DialogInputs::Height,
                            entities::DialogInputs::TopThickness,
                            entities::DialogInputs::Height
                        },
                    }},
                    {
                        {entities::Position::Outside, {
                            jointTypes{std::make_tuple(entities::JointType::Normal, axisList{entities::AxisFlag::Length, entities::AxisFlag::Width})}
                        }},
                        {entities::Position::Inside, {
                            jointTypes{std::make_tuple(entities::JointType::Inverse, axisList{entities::AxisFlag::Length, entities::AxisFlag::Width}),
                                       std::make_tuple(entities::JointType::Corner, axisList{entities::AxisFlag::Length, entities::AxisFlag::Width})}
                        }}
                    }
                },
                DimensionTableRow{
                    PanelLabelConfig{"bottomPanelLabel", "Bottom"},
                    EnabledInputConfig{entities::DialogInputs::BottomEnable, "bottomEnableInput", "Bottom", true},
                    OverrideInputConfig{entities::DialogInputs::BottomOverride, "bottomOverrideInput", "Bottom"},
                    PanelThicknessConfig{
                        entities::DialogInputs::BottomThickness, "bottom_thickness", "bottomThicknessInput", "Bottom",
                        std::unordered_map<bool, InputDefaults>{
                            {true,  InputDefaults{"mm", 0.01, 2540, .1, 3.2}},
                            {false, InputDefaults{"in", 0.005, 48, 0.0625, .125}}
                        }},
                    entities::AxisFlag::Height,
                    {{
                         adsk::core::YUpModelingOrientation, PanelReferencePointIds{
                            entities::DialogInputs::Length,
                            entities::DialogInputs::Width,
                            entities::DialogInputs::BottomThickness,
                            entities::DialogInputs::BottomThickness,
                            entities::DialogInputs::Height
                        }},{
                        adsk::core::ZUpModelingOrientation, PanelReferencePointIds{
                            entities::DialogInputs::Length,
                            entities::DialogInputs::Width,
                            entities::DialogInputs::BottomThickness,
                            entities::DialogInputs::BottomThickness,
                            entities::DialogInputs::Height
                        },
                     }},
                    {
                        {entities::Position::Outside, {
                            jointTypes{std::make_tuple(entities::JointType::Normal, axisList{entities::AxisFlag::Length, entities::AxisFlag::Width})}
                        }},
                        {entities::Position::Inside, {
                            jointTypes{std::make_tuple(entities::JointType::Inverse, axisList{entities::AxisFlag::Length, entities::AxisFlag::Width}),
                                       std::make_tuple(entities::JointType::Corner, axisList{entities::AxisFlag::Length, entities::AxisFlag::Width})}
                        }}
                     }
                },
                DimensionTableRow{
                    PanelLabelConfig{"leftPanelLabel", "Left"},
                    EnabledInputConfig{entities::DialogInputs::LeftEnable, "leftEnableInput", "Left", true},
                    OverrideInputConfig{entities::DialogInputs::LeftOverride, "leftOverrideInput", "Left"},
                    PanelThicknessConfig{
                        entities::DialogInputs::LeftThickness, "left_thickness", "leftThicknessInput", "Left",
                        std::unordered_map<bool, InputDefaults>{{true,  InputDefaults{"mm", 0.01, 2540, .1, 3.2}},
                                                                {false, InputDefaults{"in", 0.005, 48, 0.0625, .125}}}
                    },
                    entities::AxisFlag::Length,
                    {{
                         adsk::core::YUpModelingOrientation, PanelReferencePointIds{
                            entities::DialogInputs::LeftThickness,
                            entities::DialogInputs::Width,
                            entities::DialogInputs::Height,
                            entities::DialogInputs::LeftThickness,
                            entities::DialogInputs::Length
                        }},{
                         adsk::core::ZUpModelingOrientation, PanelReferencePointIds{
                            entities::DialogInputs::LeftThickness,
                            entities::DialogInputs::Width,
                            entities::DialogInputs::Height,
                            entities::DialogInputs::LeftThickness,
                            entities::DialogInputs::Length
                        },
                     }},
                    {
                        {entities::Position::Outside, {
                                jointTypes{std::make_tuple(entities::JointType::Inverse, axisList{entities::AxisFlag::Height, entities::AxisFlag::Width}),
                                    std::make_tuple(
                                        entities::JointType::Corner,
                                        axisList{entities::AxisFlag::Height, entities::AxisFlag::Width}
                                        )
                                }
                        }},
                        {entities::Position::Inside, {
                                jointTypes{std::make_tuple(entities::JointType::Normal, axisList{entities::AxisFlag::Height, entities::AxisFlag::Width})}
                            }}
                    }
                },
                DimensionTableRow{
                    PanelLabelConfig{"rightPanelLabel", "Right"},
                    EnabledInputConfig{entities::DialogInputs::RightEnable, "rightEnableInput", "Right", true},
                    OverrideInputConfig{entities::DialogInputs::RightOverride, "rightOverrideInput", "Right"},
                    PanelThicknessConfig{
                        entities::DialogInputs::RightThickness, "right_thickness", "rightThicknessInput", "Right",
                        std::unordered_map<bool, InputDefaults>{{true,  InputDefaults{"mm", 0.01, 2540, .1, 3.2}},
                                                                {false, InputDefaults{"in", 0.005, 48, 0.0625, .125}}}
                    },
                    entities::AxisFlag::Length,
                    {{
                         adsk::core::YUpModelingOrientation, PanelReferencePointIds{
                            entities::DialogInputs::Length,
                            entities::DialogInputs::Width,
                            entities::DialogInputs::Height,
                            entities::DialogInputs::RightThickness,
                            entities::DialogInputs::Length
                        }},{
                         adsk::core::ZUpModelingOrientation, PanelReferencePointIds{
                            entities::DialogInputs::Length,
                            entities::DialogInputs::Width,
                            entities::DialogInputs::Height,
                            entities::DialogInputs::RightThickness,
                            entities::DialogInputs::Length
                        },
                     }},
                    {
                        {entities::Position::Outside, {
                                jointTypes{
                                    std::make_tuple(
                                        entities::JointType::Inverse,
                                        axisList{entities::AxisFlag::Height, entities::AxisFlag::Width}
                                        ),
                                    std::make_tuple(
                                        entities::JointType::Corner,
                                        axisList{entities::AxisFlag::Height, entities::AxisFlag::Width}
                                        )
                                }
                        }},
                        {entities::Position::Inside, {
                                jointTypes{std::make_tuple(entities::JointType::Normal, axisList{entities::AxisFlag::Height, entities::AxisFlag::Width})}
                            }}
                    }
                },
                DimensionTableRow{
                    PanelLabelConfig{"frontPanelLabel", "Front"},
                    EnabledInputConfig{entities::DialogInputs::FrontEnable, "frontEnableInput", "Front", true},
                    OverrideInputConfig{entities::DialogInputs::FrontOverride, "frontOverrideInput", "Front"},
                    PanelThicknessConfig{
                        entities::DialogInputs::FrontThickness, "front_thickness", "frontThicknessInput", "Front",
                        std::unordered_map<bool, InputDefaults>{{true,  InputDefaults{"mm", 0.01, 2540, .1, 3.2}},
                                                                {false, InputDefaults{"in", 0.005, 48, 0.0625, .125}}}
                    },
                    entities::AxisFlag::Width,
                    {{
                         adsk::core::YUpModelingOrientation, PanelReferencePointIds{
                            entities::DialogInputs::Length,
                            entities::DialogInputs::Width,
                            entities::DialogInputs::Height,
                            entities::DialogInputs::FrontThickness,
                            entities::DialogInputs::Width
                        }},{
                         adsk::core::ZUpModelingOrientation, PanelReferencePointIds{
                            entities::DialogInputs::Length,
                            entities::DialogInputs::FrontThickness,
                            entities::DialogInputs::Height,
                            entities::DialogInputs::FrontThickness,
                            entities::DialogInputs::Width
                        },
                     }},
                    {
                        {entities::Position::Outside, {
                                jointTypes{
                                    std::make_tuple(
                                        entities::JointType::Normal,
                                        axisList{entities::AxisFlag::Length}
                                        ),
                                    std::make_tuple(
                                        entities::JointType::Corner,
                                        axisList{entities::AxisFlag::Height}
                                        ),
                                    std::make_tuple(
                                        entities::JointType::Inverse,
                                        axisList{entities::AxisFlag::Height}
                                        )
                                }
                        }},
                        {entities::Position::Inside, {
                                jointTypes{std::make_tuple(entities::JointType::Normal, axisList{entities::AxisFlag::Height, entities::AxisFlag::Length})}
                            }}
                    }
                },
                DimensionTableRow{
                    PanelLabelConfig{"backPanelLabel", "Back"},
                    EnabledInputConfig{entities::DialogInputs::BackEnable, "backEnableInput", "Back", true},
                    OverrideInputConfig{entities::DialogInputs::BackOverride, "backOverrideInput", "Back"},
                    PanelThicknessConfig{
                        entities::DialogInputs::BackThickness, "back_thickness", "backThicknessInput", "Back",
                        std::unordered_map<bool, InputDefaults>{{true,  InputDefaults{"mm", 0.01, 2540, .1, 3.2}},
                                                                {false, InputDefaults{"in", 0.005, 48, 0.0625, .125}}}
                    },
                    entities::AxisFlag::Width,
                    {{
                         adsk::core::YUpModelingOrientation, PanelReferencePointIds{
                            entities::DialogInputs::Length,
                            entities::DialogInputs::BackThickness,
                            entities::DialogInputs::Height,
                            entities::DialogInputs::BackThickness,
                            entities::DialogInputs::Width
                        }},{
                         adsk::core::ZUpModelingOrientation, PanelReferencePointIds{
                            entities::DialogInputs::Length,
                            entities::DialogInputs::Width,
                            entities::DialogInputs::Height,
                            entities::DialogInputs::BackThickness,
                            entities::DialogInputs::Width
                        },
                     }},
                    {
                        {entities::Position::Outside, {
                                jointTypes{
                                    std::make_tuple(
                                        entities::JointType::Normal,
                                        axisList{entities::AxisFlag::Length}
                                        ),
                                    std::make_tuple(
                                        entities::JointType::Corner,
                                        axisList{entities::AxisFlag::Height}
                                        ),
                                    std::make_tuple(
                                        entities::JointType::Inverse,
                                        axisList{entities::AxisFlag::Height}
                                        )
                                }
                        }},
                        {entities::Position::Inside, {
                                jointTypes{std::make_tuple(entities::JointType::Normal, axisList{entities::AxisFlag::Height, entities::AxisFlag::Length})}
                            }}
                    }
                }
            }
        };

        adsk::core::Ptr<adsk::core::Application> m_app;

        adsk::core::Ptr<adsk::core::TextBoxCommandInput>   m_error;
        adsk::core::Ptr<adsk::core::DropDownCommandInput>  m_creation_mode;
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> m_fast_preview;
        adsk::core::Ptr<adsk::core::TextBoxCommandInput>   m_fast_label;
        adsk::core::Ptr<adsk::core::DropDownCommandInput>  m_finger_mode;
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> m_full_preview;
        adsk::core::Ptr<adsk::core::TextBoxCommandInput>   m_full_label;

        std::unordered_map<entities::AxisFlag, std::vector<EnableThicknessPair> >                     m_axis_list;
        std::unordered_map<entities::DialogInputs, adsk::core::Ptr<adsk::core::CommandInput> >        m_controls;
        std::unordered_map<entities::DialogInputs, std::vector<std::function<void()>>>                m_handlers;
        std::unordered_map<std::string, entities::DialogInputs>                                       m_inputs;
        std::unordered_map<entities::DialogInputs, adsk::core::Ptr<adsk::core::TextBoxCommandInput> > m_labels;
        std::unordered_map<entities::DialogInputs, ParameterizedControl>                              m_parameters;
        std::vector<EnableThicknessControl>                                                           m_thicknesses;
        std::vector<std::function<bool()> >                                                           m_validators;
        std::vector<std::string>                                                                      m_ignore_updates;
        std::vector<bool>                                                                             m_results;

        std::string m_error_message;

        entt::registry &m_registry;

        auto createDimensionGroup(
            const adsk::core::Ptr<adsk::core::CommandInputs> &inputs, bool is_metric
        ) -> adsk::core::Ptr<adsk::core::GroupCommandInput>;

        void createPanelTable(
            const adsk::core::Ptr<adsk::core::CommandInputs>& inputs,
            const adsk::core::Ptr<adsk::fusion::Component>& root,
            adsk::core::DefaultModelingOrientations orientation,
            bool is_metric
        );

        void addInputControl(entities::DialogInputs reference, const adsk::core::Ptr<adsk::core::CommandInput>& input);
        void addInputControl(
            entities::DialogInputs reference,
            const adsk::core::Ptr<adsk::core::CommandInput>& input,
            const std::vector<std::function<void()>>& handlers
        );
        void addInputControl(
            entities::DialogInputs reference,
            const adsk::core::Ptr<adsk::core::CommandInput>& input,
            const std::function<void()>& handler
        );
        void addInputHandler(entities::DialogInputs reference, const std::function<void()>& handler);
        void addMinimumAxisDimensionChecks();
        void addMaximumKerfCheck();
        void addMinimumFingerWidthCheck();
        void createDividerInputs(const adsk::core::Ptr<adsk::core::CommandInputs>& inputs);
        void createFingerModeSelectionDropDown(const adsk::core::Ptr<adsk::core::CommandInputs>& inputs);
        void createPreviewTable(const adsk::core::Ptr<adsk::core::CommandInputs>& inputs);
        void createModelSelectionDropDown(const adsk::core::Ptr<adsk::core::CommandInputs>& inputs);

    public:
        explicit CreateDialog(entt::registry &registry) : m_registry(registry) {
            m_error_message = "";
        };

        void clear();

        void create(
            const adsk::core::Ptr<adsk::core::Application> &app,
            const adsk::core::Ptr<adsk::core::CommandInputs> &inputs,
            const adsk::core::Ptr<adsk::fusion::Component> &root,
            adsk::core::DefaultModelingOrientations orientation,
            bool is_metric
        );

        bool update(const adsk::core::Ptr<adsk::core::CommandInput> &cmd_input);
        bool validate(const adsk::core::Ptr<adsk::core::CommandInputs> &inputs);
        bool full_preview() { return m_full_preview->value(); };
        bool fast_preview() { return m_fast_preview->value(); };
        bool is_parametric() { return m_creation_mode->selectedItem()->index() == 0; };

        void addTableTitles(adsk::core::Ptr<adsk::core::TableCommandInput> &table) const;

        [[nodiscard]] adsk::core::Ptr<adsk::core::TableCommandInput> initializePanelTable(const adsk::core::Ptr<adsk::core::CommandInputs> &inputs) const;
    };

}

#endif /* silvanuspro_dialog_hpp */
