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

#include "dialog/entities/InputConfig.hpp"
#include "entities/AxisFlag.hpp"
#include "lib/generatebox/dialog/entities/DialogInputs.hpp"
#include "entities/FingerPattern.hpp"
#include "entities/JointPatternTags.hpp"
#include "entities/JointDirection.hpp"
#include "entities/JointPattern.hpp"
#include "entities/OrientationTags.hpp"
#include "entities/Panel.hpp"
#include "entities/Panels.hpp"
#include "entities/Position.hpp"
#include "createpaneloverriderow.hpp"

#include "lib/generatebox/dialog/systems/DialogSystemManager.hpp"

namespace silvanus::generatebox::dialog {

    struct InputDefaults {
        const std::string unit_type;
        const float       minimum;
        const float       maximum;
        const float       step;
        const float       initial_value;
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

    using jointTypes = std::vector<std::tuple<entities::JointPatternType, std::vector<entities::AxisFlag>>>;
    using jointPositions = std::map<entities::Position, jointTypes>;

    struct DimensionTableRow {
        const entities::Panels       id;
        const int                    priority;
        const PanelLabelConfig       label;
        const EnabledInputConfig     enable;
        const OverrideInputConfig    override;
        const PanelThicknessConfig   thickness;
        const entities::AxisFlag     orientation;
    };

    struct DimensionTable {
        const std::string                      id;
        const std::string                      name;
        const std::string                      column_ratio;
        const std::vector<DimensionTableTitle> titles;
        const std::vector<DimensionTableRow>   dimensions;
    };

    class CreateFusionDialog {

            InputConfig const length_metric_defaults = {
                entities::DialogInputs::Length, "length", "lengthSpinnerInput", "Length", "mm", 0.01, 2540, .1, 285
            };
            InputConfig const width_metric_defaults = {
                entities::DialogInputs::Width, "width", "widthSpinnerInput", "Width", "mm", 0.01, 2540, .1, 142.5
            };
            InputConfig const height_metric_defaults = {
                entities::DialogInputs::Height, "height", "heightSpinnerInput", "Height", "mm", 0.01, 2540, .1, 40
            };
            InputConfig const thickness_metric_defaults = {
                entities::DialogInputs::Thickness, "thickness", "thicknessSpinnerInput", "Thickness", "mm", 0.01, 2540, .1, 3.2
            };
            InputConfig const finger_metric_defaults = {
                entities::DialogInputs::FingerWidth, "finger_width", "fingerWidthSpinnerInput", "Finger Width", "mm", 0.01, 50.4,.1, 9.6
            };
            InputConfig const kerf_metric_defaults = {
                entities::DialogInputs::Kerf, "kerf", "kerfSpinnerInput", "Kerf", "mm", 0, 25.4, .05, 0
            };

            InputConfig const length_imperial_defaults = {
                entities::DialogInputs::Length, "length", "lengthSpinnerInput", "Length", "in", 0.005, 48, 0.0625, 12
            };
            InputConfig const width_imperial_defaults = {
                entities::DialogInputs::Width, "width", "widthSpinnerInput", "Width", "in", 0.005, 48, 0.0625, 6
            };
            InputConfig const height_imperial_defaults = {
                entities::DialogInputs::Height, "height", "heightSpinnerInput", "Height", "in", 0.005, 48, 0.0625, 3.5
            };
            InputConfig const thickness_imperial_defaults = {
                entities::DialogInputs::Thickness, "thickness", "thicknessSpinnerInput", "Thickness", "in", 0.005, 48, 0.0625, .125
            };
            InputConfig const finger_imperial_defaults = {
                entities::DialogInputs::FingerWidth, "finger_width", "fingerWidthSpinnerInput", "Finger Width", "in", 0.005, 48, 0.0625, .375
            };
            InputConfig const kerf_imperial_defaults = {
                entities::DialogInputs::Kerf, "kerf", "kerfSpinnerInput", "Kerf", "in", 0, 1, .05, 0
            };

            adsk::core::Ptr<adsk::core::Application> m_app;

            adsk::core::Ptr<adsk::core::TextBoxCommandInput> m_error;

            std::unordered_map<std::string, std::vector<std::function<void()>>>                           m_handlers;
            std::unordered_map<entities::DialogInputs, std::string>                                       m_inputs;
            std::vector<std::function<bool()> >                                                           m_validators;
            std::vector<std::string>                                                                      m_ignore_updates;
            std::vector<bool>                                                                             m_results;
            std::unique_ptr<DialogSystemManager>                                                          m_systems;

            entt::registry m_configuration = entt::registry{};
            entt::registry &m_panel_registry;

            using dimensionConfig = std::unordered_map<bool, InputConfig>;
            dimensionConfig length_defaults = {
                {true, length_metric_defaults},
                {false, length_imperial_defaults}
            };

            dimensionConfig width_defaults = {
                {true, width_metric_defaults},
                {false, width_imperial_defaults}
            };

            dimensionConfig height_defaults = {
                {true, height_metric_defaults},
                {false, height_imperial_defaults}
            };

            dimensionConfig thickness_defaults = {
                {true, thickness_metric_defaults},
                {false, thickness_imperial_defaults}
            };

            dimensionConfig finger_defaults = {
                {true, finger_metric_defaults},
                {false, finger_imperial_defaults}
            };

            dimensionConfig kerf_defaults = {
                {true, kerf_metric_defaults},
                {false, kerf_imperial_defaults}
            };

            DimensionTableRow m_top_row =                 {
                entities::Panels::Top,
                0,
                {"topPanelLabel", "Top"},
                {entities::DialogInputs::TopEnable, "topEnableInput", "Top", false},
                {entities::DialogInputs::TopOverride, "topOverrideInput", "Top"},
                {
                    entities::DialogInputs::TopThickness, "top_thickness", "topThicknessInput", "Top",
                    std::unordered_map<bool, InputDefaults>{
                        {true,  {"mm", 0.01, 2540, .1, 3.2}},
                        {false, {"in", 0.005, 48, 0.0625, .125}}
                    }
                },
                entities::AxisFlag::Height
            };

            DimensionTableRow m_bottom_row =                 {
                entities::Panels::Bottom,
                0,
                {"bottomPanelLabel", "Bottom"},
                {entities::DialogInputs::BottomEnable, "bottomEnableInput", "Bottom", true},
                {entities::DialogInputs::BottomOverride, "bottomOverrideInput", "Bottom"},
                {
                    entities::DialogInputs::BottomThickness, "bottom_thickness", "bottomThicknessInput", "Bottom",
                    std::unordered_map<bool, InputDefaults>{
                        {true,  {"mm", 0.01, 2540, .1, 3.2}},
                        {false, {"in", 0.005, 48, 0.0625, .125}}
                    }
                },
                entities::AxisFlag::Height
            };

            DimensionTableRow m_left_row =                 {
                entities::Panels::Left,
                2,
                {"leftPanelLabel", "Left"},
                {entities::DialogInputs::LeftEnable, "leftEnableInput", "Left", true},
                {entities::DialogInputs::LeftOverride, "leftOverrideInput", "Left"},
                {
                    entities::DialogInputs::LeftThickness, "left_thickness", "leftThicknessInput", "Left",
                    std::unordered_map<bool, InputDefaults>{{true,  {"mm", 0.01, 2540, .1, 3.2}},
                                                            {false, {"in", 0.005, 48, 0.0625, .125}}}
                },
                entities::AxisFlag::Length
            };

            DimensionTableRow m_right_row = {
                entities::Panels::Right,
                2,
                {"rightPanelLabel", "Right"},
                {entities::DialogInputs::RightEnable, "rightEnableInput", "Right", true},
                {entities::DialogInputs::RightOverride, "rightOverrideInput", "Right"},
                {
                    entities::DialogInputs::RightThickness, "right_thickness", "rightThicknessInput", "Right",
                    std::unordered_map<bool, InputDefaults>{{true,  {"mm", 0.01, 2540, .1, 3.2}},
                                                            {false, {"in", 0.005, 48, 0.0625, .125}}}
                },
                entities::AxisFlag::Length
            };

            DimensionTableRow m_front_row = {
                entities::Panels::Front,
                1,
                {"frontPanelLabel", "Front"},
                {entities::DialogInputs::FrontEnable, "frontEnableInput", "Front", true},
                {entities::DialogInputs::FrontOverride, "frontOverrideInput", "Front"},
                {
                    entities::DialogInputs::FrontThickness, "front_thickness", "frontThicknessInput", "Front",
                    std::unordered_map<bool, InputDefaults>{{true,  {"mm", 0.01, 2540, .1, 3.2}},
                                                            {false, {"in", 0.005, 48, 0.0625, .125}}}
                },
                entities::AxisFlag::Width
            };

            DimensionTableRow m_back_row = {
                entities::Panels::Back,
                1,
                {"backPanelLabel", "Back"},
                {entities::DialogInputs::BackEnable, "backEnableInput", "Back", true},
                {entities::DialogInputs::BackOverride, "backOverrideInput", "Back"},
                {
                    entities::DialogInputs::BackThickness, "back_thickness", "backThicknessInput", "Back",
                    std::unordered_map<bool, InputDefaults>{{true,  {"mm", 0.01, 2540, .1, 3.2}},
                                                            {false, {"in", 0.005, 48, 0.0625, .125}}}
                },
                entities::AxisFlag::Width
            };

            DimensionTable m_dimensions_table = {
                "dimensionsTableInput", "Dimensions", "4:1:2:1:1:2:1:5",
                {
                    {"panelTitleTextInput", "Panel", "Panel", 0, 0},
                    {"enableTitleTextInput", "Enable", "Enable", 1, 2},
                    {"overrideTitleTextInput", "Override", "Override", 4, 2},
                    {"thicknessTitleTextInput", "Thickness", "Thickness", 7, 0}
                },
                {
                    m_top_row,
                    m_bottom_row,
                    m_left_row,
                    m_right_row,
                    m_front_row,
                    m_back_row
                }
            };

            void addMinimumAxisDimensionChecks();
            void addMaximumKerfCheck();
            void addMinimumFingerWidthCheck();
            void addMinimumPanelCountCheck();

            template <class T, class U>
            auto addPanelTableRow(
                const adsk::core::Ptr<adsk::core::CommandInputs> &inputs,
                adsk::core::Ptr<adsk::core::TableCommandInput> &table,
                const DimensionTableRow &row
            ) -> CreatePanelOverrideRow;

            void addTableTitles(adsk::core::Ptr<adsk::core::TableCommandInput>& table) const;
            auto createDimensionGroup(
                const adsk::core::Ptr<adsk::core::CommandInputs> &inputs
            ) -> adsk::core::Ptr<adsk::core::GroupCommandInput>;
            auto createDimensionInput(
                adsk::core::Ptr<adsk::core::CommandInputs>& children, const InputConfig& config
            ) -> adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput>;
            void createDividerInputs(const adsk::core::Ptr<adsk::core::CommandInputs>& inputs);
            void createFingerModeSelectionDropDown(const adsk::core::Ptr<adsk::core::CommandInputs>& inputs);
            auto createStandardJointTable(
                const adsk::core::Ptr<adsk::core::CommandInputs>& inputs
            ) -> adsk::core::Ptr<adsk::core::TableCommandInput>;
            void createPanelTable(
                const adsk::core::Ptr<adsk::core::CommandInputs>& inputs
            );
            void createPreviewTable(const adsk::core::Ptr<adsk::core::CommandInputs>& inputs);
            void createModelSelectionDropDown(const adsk::core::Ptr<adsk::core::CommandInputs>& inputs);

            [[nodiscard]] adsk::core::Ptr<adsk::core::TableCommandInput> initializePanelTable(const adsk::core::Ptr<adsk::core::CommandInputs> &inputs) const;

            static auto addPanelLabelControl(
                const adsk::core::Ptr<adsk::core::CommandInputs> &inputs, const DimensionTableRow &row
            ) -> adsk::core::Ptr<adsk::core::TextBoxCommandInput>;
            static auto addPanelEnableControl(
                const adsk::core::Ptr<adsk::core::CommandInputs> &inputs, const DimensionTableRow &row
            ) -> adsk::core::Ptr<adsk::core::BoolValueCommandInput>;
            static auto addPanelOverrideControl(
                const adsk::core::Ptr<adsk::core::CommandInputs> &inputs, const DimensionTableRow &row
            ) -> adsk::core::Ptr<adsk::core::BoolValueCommandInput>;

            void populateJointTable(adsk::core::Ptr<adsk::core::TableCommandInput>& table);

            // Post-dialog ECS systems

        public:
            explicit CreateFusionDialog(entt::registry& registry) : m_panel_registry{ registry } {
                m_systems = std::make_unique<DialogSystemManager>(m_configuration);
                m_configuration.set<entities::DialogErrorMessage>("");
            };

            void clear();

            void create(
                const adsk::core::Ptr<adsk::core::Application> &app,
                const adsk::core::Ptr<adsk::core::CommandInputs> &inputs,
                const adsk::core::Ptr<adsk::fusion::Component> &root,
                adsk::core::DefaultModelingOrientations orientation,
                bool is_metric
            );

            void initializePanels();

            bool update(const adsk::core::Ptr<adsk::core::CommandInput> &cmd_input);
            bool validate(const adsk::core::Ptr<adsk::core::CommandInputs> &inputs);
            bool full_preview() { return m_configuration.ctx<entities::DialogFullPreviewMode>().control->value(); };
            bool fast_preview() { return m_configuration.ctx<entities::DialogFastPreviewMode>().control->value(); };
            bool is_parametric() { return m_configuration.ctx<entities::DialogCreationMode>().control->selectedItem()->index() == 0; };

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
            void addCollisionHandler(entities::DialogInputs reference);
            static void addJointTypes(adsk::core::Ptr<adsk::core::DropDownCommandInput> &type_dropdown) ;

            void createDividerOrientationsInput(const adsk::core::Ptr<adsk::core::CommandInputs> &inputs);
            void createDividerJointDirectionInput(const adsk::core::Ptr<adsk::core::CommandInputs> &inputs);
            void createLengthDividerInputs(const adsk::core::Ptr<adsk::core::CommandInputs> &inputs);
            void createWidthDividerInputs(const adsk::core::Ptr<adsk::core::CommandInputs> &inputs);
            void createHeightDividerInputs(const adsk::core::Ptr<adsk::core::CommandInputs> &inputs);
    };

}

#endif /* silvanuspro_dialog_hpp */
