//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/22/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//

#include "dialog.hpp"

#include "dividers.hpp"

#include "entities/AxisFlag.hpp"
#include "entities/FingerMode.hpp"
#include "entities/JointType.hpp"
#include "entities/Dimensions.hpp"
#include "entities/Enabled.hpp"
#include "entities/EnableInput.hpp"
#include "entities/EndReferencePoint.hpp"
#include "entities/FingerWidth.hpp"
#include "entities/FingerWidthInput.hpp"
#include "entities/FingerPatternType.hpp"
#include "entities/HeightJointInput.hpp"
#include "entities/InsidePanel.hpp"
#include "entities/JointName.hpp"
#include "entities/JointOrientation.hpp"
#include "entities/JointPatternDistance.hpp"
#include "entities/JointPatternTags.hpp"
#include "entities/JointPatternType.hpp"
#include "entities/JointProfile.hpp"
#include "entities/JointThickness.hpp"
#include "entities/KerfInput.hpp"
#include "entities/LengthJointInput.hpp"
#include "entities/MaxOffset.hpp"
#include "entities/MaxOffsetInput.hpp"
#include "entities/OrientationGroup.hpp"
#include "entities/OrientationTags.hpp"
#include "entities/OutsidePanel.hpp"
#include "entities/OverrideInput.hpp"
#include "entities/PanelId.hpp"
#include "entities/PanelName.hpp"
#include "entities/PanelOrientation.hpp"
#include "entities/PanelPosition.hpp"
#include "entities/PanelProfile.hpp"
#include "entities/PanelType.hpp"
#include "entities/Position.hpp"
#include "entities/StartReferencePoint.hpp"
#include "entities/ToggleableThicknessInput.hpp"
#include "entities/ThicknessInput.hpp"
#include "entities/WidthJointInput.hpp"

#include <numeric>

using std::accumulate;
using std::all_of;
using std::get;
using std::vector;

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox;

void CreateDialog::clear() {
    m_controls.clear();
    m_axis_list.clear();
    m_validators.clear();
    m_handlers.clear();
    m_results.clear();
    m_inputs.clear();
    m_labels.clear();
    m_parameters.clear();
    m_thicknesses.clear();
    m_ignore_updates.clear();
    m_panels.clear();
    m_configuration.clear();
}

void CreateDialog::create(const adsk::core::Ptr<Application> &app, const adsk::core::Ptr<CommandInputs> &inputs, const adsk::core::Ptr<Component> &root,
                          DefaultModelingOrientations orientation,
                          bool is_metric) {
    m_app = app;

    auto const& dimensions = inputs->addTabCommandInput("dimensionsTabInput", "", "resources/dimensions");
    dimensions->tooltip("Dimensions");

    auto dimension_children = dimensions->children();

    createModelSelectionDropDown(dimension_children);
    createFingerModeSelectionDropDown(dimension_children);

    auto dimensions_group = createDimensionGroup(dimension_children, is_metric);
    createPanelTable(dimensions_group->children(), root, orientation, is_metric);

    auto const& dividers = inputs->addTabCommandInput("dividersTabInput", "", "resources/dividers");
    dividers->tooltip("Dividers");
    createDividerInputs(dividers->children());

    createPreviewTable(inputs);
    m_error = inputs->addTextBoxCommandInput("errorMessageCommandInput", "", "", 2, true);
    m_error->isVisible(false);

    m_ignore_updates.emplace_back(m_error->id());
    addMinimumAxisDimensionChecks();
    addMaximumKerfCheck();
    addMinimumFingerWidthCheck();
}

void CreateDialog::createModelSelectionDropDown(const Ptr<CommandInputs>& inputs) {
    m_creation_mode = inputs->addDropDownCommandInput("creationTypeCommandInput", "Type of Model", TextListDropDownStyle);
    auto const& creation_items = m_creation_mode->listItems();
    creation_items->add("Parametric", true);
    creation_items->add("Direct Model", false);
    m_creation_mode->maxVisibleItems(2);

    addInputControl(DialogInputs::ModelSelection, m_creation_mode, [this](){
        auto const& full = Ptr<BoolValueCommandInput>{m_controls[DialogInputs::FullPreview]};
        auto const& label = Ptr<TextBoxCommandInput>{m_controls[DialogInputs::FullPreviewLabel]};

        if (full->value()) {
            full->value(false);
        }

        auto const is_parametric_mode = this->m_creation_mode->selectedItem()->index() == 0;
        full->isVisible(is_parametric_mode);
        label->isVisible(is_parametric_mode);
    });
}

void CreateDialog::createFingerModeSelectionDropDown(const Ptr<CommandInputs>& inputs) {
    m_finger_mode = inputs->addDropDownCommandInput("fingerTypeCommandInput", "Finger Size", TextListDropDownStyle);
    auto const& creation_items = m_finger_mode->listItems();
    creation_items->add("Automatic Width", true);
    creation_items->add("Constant Width", false);
    creation_items->add("None", false);
    m_finger_mode->maxVisibleItems(3);

    addInputControl(DialogInputs::FingerMode, m_finger_mode);
}

void CreateDialog::createDividerInputs(const Ptr<CommandInputs>& inputs) {
    m_divider_joint = inputs->addDropDownCommandInput("dividerLapCommandInput", "Top Joint", TextListDropDownStyle);
    auto const& joint_items = m_divider_joint->listItems();
    joint_items->add("Length Dividers", false);
    joint_items->add("Width Dividers", true);
    m_divider_joint->maxVisibleItems(2);
    addInputControl(DialogInputs::DividerLapInput, m_divider_joint, [this](){
        this->update(m_controls[DialogInputs::LengthDividerCount]);
        this->update(m_controls[DialogInputs::WidthDividerCount]);
    });

    m_length_divider_outside_joint = inputs->addDropDownCommandInput("lengthDividerOutsideJointInput", "Length Divider Joint", TextListDropDownStyle);
    auto const& length_items = m_length_divider_outside_joint->listItems();
    length_items->add("Tenon", true);
    length_items->add("Half Lap", false);
    length_items->add("Box Joint", false);
    m_length_divider_outside_joint->maxVisibleItems(3);
    addInputControl(DialogInputs::LengthDividerJointInput, m_length_divider_outside_joint, [this](){
        this->update(m_controls[DialogInputs::LengthDividerCount]);
    });

    m_panel_registry.view<JointLengthOrientation, InsideJointPattern, OutsidePanel>().each([this](
        auto entity, auto const& orientation, auto const& pattern_position, auto const& panel_position
    ){
        this->m_panel_registry.emplace<LengthJointInput>(entity, this->m_length_divider_outside_joint);
    });

    auto length = adsk::core::Ptr<IntegerSpinnerCommandInput>{inputs->addIntegerSpinnerCommandInput(
        "lengthDividerCommandInput", "(#) Length Dividers", 0, 25, 1, 0
    )};
    addInputControl(DialogInputs::LengthDividerCount, length, [this](){

        auto old_view = this->m_panel_registry.view<InsidePanel, LengthOrientation>();
        this->m_panel_registry.destroy(old_view.begin(), old_view.end());

        auto const outside_joint_type = this->m_length_divider_outside_joint->selectedItem()->index();
        auto const inside_joint_type = this->m_divider_joint->selectedItem()->index();
        auto joint_selector = std::map<int, JointType>{
            {0, JointType::TopLap},
            {1, JointType::BottomLap}
        };
        auto joints = addInsideJoints(AxisFlag::Length, joint_selector[inside_joint_type], outside_joint_type);

        auto dividers = Dividers<LengthOrientation>(this->m_panel_registry, this->m_app, this->m_controls, joints);
        dividers.create("Length", DialogInputs::LengthDividerCount, DialogInputs::Length, AxisFlag::Length);
    });

    m_width_divider_outside_joint = inputs->addDropDownCommandInput("widthDividerOutsideJointInput", "Width Divider Joint", TextListDropDownStyle);
    auto const& width_items = m_width_divider_outside_joint->listItems();
    width_items->add("Tenon", true);
    width_items->add("Half Lap", false);
    width_items->add("Box Joint", false);
    m_width_divider_outside_joint->maxVisibleItems(3);
    addInputControl(DialogInputs::WidthDividerJointInput, m_width_divider_outside_joint, [this](){
        this->update(m_controls[DialogInputs::WidthDividerCount]);
    });

    m_panel_registry.view<JointWidthOrientation, InsideJointPattern, OutsidePanel>().each([this](
        auto entity, auto const& orientation, auto const& pattern_position, auto const& panel_position
    ){
        this->m_panel_registry.emplace<WidthJointInput>(entity, this->m_width_divider_outside_joint);
    });

    auto width = adsk::core::Ptr<IntegerSpinnerCommandInput>{inputs->addIntegerSpinnerCommandInput(
        "widthDividerCommandInput", "(#) Width Dividers", 0, 25, 1, 0
    )};
    addInputControl(DialogInputs::WidthDividerCount, width, [this](){

        auto old_view = this->m_panel_registry.view<InsidePanel, WidthOrientation>();
        this->m_panel_registry.destroy(old_view.begin(), old_view.end());

        auto const outside_joint_type = this->m_width_divider_outside_joint->selectedItem()->index();
        auto const inside_joint_type = this->m_divider_joint->selectedItem()->index();
        auto joint_selector = std::map<int, JointType>{
            {0, JointType::BottomLap},
            {1, JointType::TopLap}
        };
        auto joints = addInsideJoints(AxisFlag::Width, joint_selector[inside_joint_type], outside_joint_type);

        auto dividers = Dividers<WidthOrientation>(this->m_panel_registry, this->m_app, this->m_controls, joints);
        dividers.create("Width", DialogInputs::WidthDividerCount, DialogInputs::Width, AxisFlag::Width);
    });

//    auto height = adsk::core::Ptr<IntegerSpinnerCommandInput>{inputs->addIntegerSpinnerCommandInput(
//        "heightDividerCommandInput", "(#) Height Dividers", 0, 25, 1, 0
//    )};
//    m_panel_registry.view<JointHeightOrientation>().each([this](
//        auto entity
//    ){
//        this->m_panel_registry.emplace<HeightJointInput>(entity);
//    });
//
//    addInputControl(DialogInputs::HeightDividerCount, height, [this](){
//
//        auto old_view = this->m_panel_registry.view<InsidePanel, HeightOrientation>();
//        this->m_panel_registry.destroy(old_view.begin(), old_view.end());
//
//        auto dividers = Dividers<HeightOrientation>(this->m_panel_registry, this->m_app, this->m_controls);
//        dividers.create("Height", DialogInputs::HeightDividerCount, DialogInputs::Height, AxisFlag::Height);
//    });

    addInputHandler(DialogInputs::Length, [this](){
        this->update(m_controls[DialogInputs::LengthDividerCount]);
        this->update(m_controls[DialogInputs::WidthDividerCount]);
    });

    addInputHandler(DialogInputs::Width, [this](){
        this->update(m_controls[DialogInputs::LengthDividerCount]);
        this->update(m_controls[DialogInputs::WidthDividerCount]);
    });

    addInputHandler(DialogInputs::Height, [this](){
        this->update(m_controls[DialogInputs::LengthDividerCount]);
        this->update(m_controls[DialogInputs::WidthDividerCount]);
    });

}

axisJointTypePositionMap CreateDialog::addInsideJoints(
    const AxisFlag panel_orientation,
    const JointType inside_joint_type,
    const int outside_joint_type
) {
    using jointTypeMap = std::map<entities::JointType, std::vector<entities::Position>>;
    using selectorJointTypeMap = std::map<int, jointTypeMap>;
    using axisSelectorMap = std::map<AxisFlag, selectorJointTypeMap>;

    auto finger_orientation = axisJointTypePositionMap{
    };

    auto inverse_toplap_selector = selectorJointTypeMap{
        {2, {
            {JointType::Inverse, {Position::Outside}},
            {JointType::Corner, {Position::Outside}}
        }},
        {1, {
            {JointType::TopLap, {Position::Outside}},
        }},
        {0, {
            {JointType::Tenon, {Position::Outside}},
        }}
    };
    auto inverse_trim_selector = selectorJointTypeMap{
        {2, {
                {JointType::Inverse, {Position::Outside}},
                {JointType::Corner, {Position::Outside}}
            }},
        {1, {
                {JointType::Trim, {Position::Outside}},
            }},
        {0, {
                {JointType::Trim, {Position::Outside}},
            }}
    };
    auto hgt_outside_joint_selector = axisSelectorMap{
        {AxisFlag::Length, inverse_trim_selector},
        {AxisFlag::Width, inverse_trim_selector},
        {AxisFlag::Height, inverse_toplap_selector}
    };
    auto lw_outside_joint_selector = axisSelectorMap{
        {AxisFlag::Length, inverse_toplap_selector},
        {AxisFlag::Width, inverse_toplap_selector},
        {AxisFlag::Height, inverse_trim_selector}
    };

    if (panel_orientation == AxisFlag::Length) {
        finger_orientation[AxisFlag::Height] = hgt_outside_joint_selector[panel_orientation][outside_joint_type];
        finger_orientation[AxisFlag::Width] = lw_outside_joint_selector[panel_orientation][outside_joint_type];
        finger_orientation[AxisFlag::Width][inside_joint_type].emplace_back(Position::Inside);
    } else if (panel_orientation == AxisFlag::Width) {
        finger_orientation[AxisFlag::Height] = hgt_outside_joint_selector[panel_orientation][outside_joint_type];
        finger_orientation[AxisFlag::Length] = lw_outside_joint_selector[panel_orientation][outside_joint_type];
        finger_orientation[AxisFlag::Length][inside_joint_type].emplace_back(Position::Inside);
    } else if (panel_orientation == AxisFlag::Height) {
        finger_orientation[AxisFlag::Length][inside_joint_type].emplace_back(Position::Inside);
        finger_orientation[AxisFlag::Length] = hgt_outside_joint_selector[panel_orientation][outside_joint_type];
        finger_orientation[AxisFlag::Width] = hgt_outside_joint_selector[panel_orientation][outside_joint_type];
    }

    return finger_orientation;
}

void CreateDialog::createPreviewTable(const adsk::core::Ptr<CommandInputs>& inputs) {
    auto table = inputs->addTableCommandInput(
        "previewTableCommandInput", "Preview", 0, "1:5:1:5"
    );

    table->maximumVisibleRows((int) 1);
    table->minimumVisibleRows((int) 1);
    table->isEnabled(false);
    table->tablePresentationStyle(transparentBackgroundTablePresentationStyle);

    m_fast_preview = table->commandInputs()->addBoolValueInput("fastPreviewCommandInput", "Fast Preview", true, "", false);
    m_fast_label = table->commandInputs()->addTextBoxCommandInput("fastPreviewLabelInput", "Fast Preview", "Fast Preview", 1, true);
    m_full_preview = table->commandInputs()->addBoolValueInput("fullPreviewCommandInput", "Full Preview", true, "", false);
    m_full_label = table->commandInputs()->addTextBoxCommandInput("fastPreviewLabelInput", "Full Preview", "Full Preview", 1, true);

    table->addCommandInput(m_fast_preview, 0, 0);
    table->addCommandInput(m_fast_label, 0, 1);
    table->addCommandInput(m_full_preview, 0, 2);
    table->addCommandInput(m_full_label, 0, 3);

    addInputControl(DialogInputs::FastPreviewLabel, m_fast_label);
    addInputControl(DialogInputs::FastPreview, m_fast_preview, [this](){
        if (this->m_fast_preview->value()) {
            this->m_full_preview->value(false);
        }
    });

    addInputControl(DialogInputs::FullPreviewLabel, m_full_label);
    addInputControl(DialogInputs::FullPreview, m_full_preview, [this](){
        if (this->m_full_preview->value()) {
            this->m_fast_preview->value(false);
        }
    });
}

void CreateDialog::addInputControl(const DialogInputs reference, const adsk::core::Ptr<CommandInput>& input) {
    m_inputs[input->id()] = reference;
    m_controls[reference] = input;
}

void CreateDialog::addInputControl(const DialogInputs reference, const adsk::core::Ptr<CommandInput>& input, const std::function<void()>& handler) {
    addInputControl(reference, input);
    m_handlers[reference].emplace_back(handler);
}

void CreateDialog::addInputControl(
    const DialogInputs reference, const adsk::core::Ptr<CommandInput>& input,
    const std::vector<std::function<void()>>& handlers
) {
    addInputControl(reference, input);
    for (auto const& handler: handlers) {
        m_handlers[reference].emplace_back(handler);
    }
}

void CreateDialog::addInputHandler(const DialogInputs reference, const std::function<void()>& handler) {
    m_handlers[reference].emplace_back(handler);
}


auto CreateDialog::createDimensionGroup(const adsk::core::Ptr<CommandInputs>& inputs, const bool is_metric) -> adsk::core::Ptr<GroupCommandInput> {
    auto group = inputs->addGroupCommandInput("dimensionsGroupInput", "Dimensions");
    
    auto children = group->children();

    for (const auto& config: m_dimension_inputs[is_metric]) {
        auto spinner = adsk::core::Ptr<FloatSpinnerCommandInput>{children->addFloatSpinnerCommandInput(
                config.id, config.name, config.unit_type, config.minimum,
                config.maximum, config.step, config.initial_value
        )};

        auto validator = [spinner]{
            return spinner->value() >= spinner->minimumValue();
        };
        m_validators.emplace_back(validator);

        m_parameters[config.lookup] = ParameterizedControl{config.parameter, spinner};

        addInputControl(config.lookup, spinner);
    }

    return group;
}

bool CreateDialog::update(const adsk::core::Ptr<CommandInput>& cmd_input) {
    auto handlers = m_handlers[m_inputs[cmd_input->id()]];

    for (auto &handler: handlers) {
        handler();
    }
    return true;
}

bool CreateDialog::validate(const adsk::core::Ptr<CommandInputs>& inputs) {
    auto results = all_of(m_validators.begin(), m_validators.end(), [](const std::function<bool()>& v){ return v(); });

    m_error->formattedText(m_error_message);
    m_error->isVisible(!results);

    return results;
}

void CreateDialog::createPanelTable(
        const adsk::core::Ptr<CommandInputs>& inputs,
        const adsk::core::Ptr<Component>& root,
        const DefaultModelingOrientations model_orientation,
        bool is_metric
) {
    adsk::core::Ptr<TableCommandInput> table = initializePanelTable(inputs);

    auto const default_thickness = m_controls[DialogInputs::Thickness];
    for (const auto& row: m_dimensions_table.dimensions) {
        auto row_num = table->rowCount();

        auto label_control = inputs->addTextBoxCommandInput(
                row.label.id, row.label.name, "<b>" + row.label.name + "</b>", 1, true
        );
        auto enable_control = inputs->addBoolValueInput(
            row.enable.id, row.enable.name, true, "", row.enable.default_value
        );
        auto override_control = inputs->addBoolValueInput(
            row.override.id, row.override.name, true,
            "", false
        );
        override_control->isEnabled(enable_control->value());

        const auto& defaults = row.thickness.defaults.at(is_metric);

        auto thickness_control = inputs->addFloatSpinnerCommandInput(
                row.thickness.id, "", defaults.unit_type, defaults.minimum,
                defaults.maximum, defaults.step, defaults.initial_value
        );
        thickness_control->isEnabled(enable_control->value() && override_control->value());

        table->addCommandInput(label_control, row_num, 0, 0, 0);
        table->addCommandInput(enable_control, row_num, 2, 0, 0);
        table->addCommandInput(override_control, row_num, 5, 0, 0);
        table->addCommandInput(thickness_control, row_num, 7, 0, 0);

        auto thickness_swap = [this, row, override_control, thickness_control, default_thickness]{
            auto t_control = adsk::core::Ptr<CommandInput>{thickness_control};
            auto d_control = adsk::core::Ptr<CommandInput>{default_thickness};
            auto toggle = override_control->value() ? t_control : d_control;
            this->m_controls[row.thickness.lookup] = toggle;
        };

        auto thickness_toggle = [override_control, thickness_control]{
            thickness_control->isEnabled(override_control->value() && override_control->isEnabled());
        };

        auto override_toggle = [override_control, enable_control]{
            override_control->isEnabled(enable_control->value());
        };

        auto follow_thickness = [this, override_control, default_thickness, thickness_control]{
            auto d_thickness = adsk::core::Ptr<FloatSpinnerCommandInput>{default_thickness};
            auto thickness_enabled = override_control->value();

            if (thickness_enabled)
                return;

            thickness_control->value(d_thickness->value());
        };

        m_axis_list[row.orientation].emplace_back(EnableThicknessPair{enable_control, thickness_control});
        m_thicknesses.emplace_back(
            EnableThicknessControl{enable_control, override_control, thickness_control, default_thickness}
        );

        addInputControl(row.override.lookup, override_control, {thickness_swap, thickness_toggle, follow_thickness});
        addInputControl(row.enable.lookup, enable_control, {thickness_toggle, override_toggle});
        addInputControl(row.thickness.lookup, thickness_control);
        addInputHandler(DialogInputs::Thickness, follow_thickness);

        auto const& references = row.reference.at(model_orientation);

        for (auto const& [joint_position, joint_types]: row.fingers) {
            for (auto const& joint_type_map: joint_types) {
                auto joint_pos = joint_position;
                auto joint_type = std::get<JointType>(joint_type_map);
                auto finger_orientations = std::get<std::vector<AxisFlag>>(joint_type_map);

                for (auto finger_orientation: finger_orientations) {
                    auto entity = m_panel_registry.create();

                    m_panel_registry.emplace<EnableInput>(entity, enable_control);

                    m_panel_registry.emplace<DimensionsInputs>(
                        entity, m_controls[references.length], m_controls[references.width], m_controls[references.height]
                    );
                    m_panel_registry.emplace<Dimensions>(entity);

                    m_panel_registry.emplace<EndReferencePoint>(entity);
                    m_panel_registry.emplace<ExtrusionDistance>(entity);

                    m_panel_registry.emplace<FingerPatternInput>(entity, m_controls[DialogInputs::FingerMode]);
                    m_panel_registry.emplace<FingerPatternType>(entity, FingerMode::Automatic);
                    m_panel_registry.emplace<FingerWidth>(entity);
                    m_panel_registry.emplace<FingerWidthInput>(entity, m_controls[DialogInputs::FingerWidth]);

                    m_panel_registry.emplace<JointName>(entity, row.label.name);
                    m_panel_registry.emplace<JointOrientation>(entity, finger_orientation);
                    m_panel_registry.emplace<JointPanelOffset>(entity);
                    m_panel_registry.emplace<JointPatternDistance>(entity);
                    m_panel_registry.emplace<JointPatternPosition>(entity, Position::Outside, row.orientation, joint_type, finger_orientation, joint_position);
                    m_panel_registry.emplace<JointProfile>(
                        entity, Position::Outside, joint_position, joint_type, FingerMode::Automatic, 0, 0.0, 0.0, 0.0, 0.0, finger_orientation, row.orientation
                    );
                    m_panel_registry.emplace<JointThickness>(entity);

                    m_panel_registry.emplace<KerfInput>(entity, m_controls[DialogInputs::Kerf]);

                    m_panel_registry.emplace<MaxOffset>(entity);
                    m_panel_registry.emplace<MaxOffsetInput>(entity, references.max, m_controls[references.max]);

                    m_panel_registry.emplace<OrientationGroup>(entity, row.orientation, finger_orientation);
                    m_panel_registry.emplace<OutsidePanel>(entity);
                    m_panel_registry.emplace<OverrideInput>(entity, override_control);

                    m_panel_registry.emplace<PanelId>(entity, row.id);
                    m_panel_registry.emplace<PanelName>(entity, row.label.name);
                    m_panel_registry.emplace<PanelOffset>(entity);
                    m_panel_registry.emplace<PanelOrientation>(entity, row.orientation);
                    m_panel_registry.emplace<PanelPosition>(entity, Position::Outside);
                    m_panel_registry.emplace<PanelProfile>(entity);

                    m_panel_registry.emplace<StartReferencePoint>(entity);

                    m_panel_registry.emplace<ToggleableThicknessInput>(
                            entity, override_control, references.thickness, thickness_control, DialogInputs::Thickness, default_thickness
                    );

                    panel_orientation_selector[row.orientation](entity);
                    finger_orientation_selector[finger_orientation](entity);
                    joint_type_selector[joint_type](joint_pos, entity);
                    joint_pos_selector[joint_position](joint_type, entity);
                }
            }
        }
    }
}

adsk::core::Ptr<TableCommandInput> CreateDialog::initializePanelTable(const adsk::core::Ptr<CommandInputs> &inputs) const {
    auto table = inputs->addTableCommandInput(
        m_dimensions_table.id, m_dimensions_table.name, 0, m_dimensions_table.column_ratio
    );

    auto const num_rows = m_dimensions_table.dimensions.size() + 1;
    table->maximumVisibleRows((int) num_rows);
    table->minimumVisibleRows((int) num_rows);
    table->isEnabled(false);
    table->tablePresentationStyle(transparentBackgroundTablePresentationStyle);

    addTableTitles(table);
    return table;
}

void CreateDialog::addTableTitles(adsk::core::Ptr<TableCommandInput> &table) const {
    for (const auto & title: m_dimensions_table.titles) {
        auto title_input = table->commandInputs()->addTextBoxCommandInput(
                title.id, title.name, "<b>" + title.label + "</b>", 1, true
        );
        table->addCommandInput(title_input, 0, title.column, 0, title.span);
    }
}

void CreateDialog::addMinimumAxisDimensionChecks() {
    for (auto pair: m_axis_to_dimension) {
        auto dimension_id = get<DialogInputs>(pair);
        auto axis_flag = get<AxisFlag>(pair);

        auto validator = [this, axis_flag, dimension_id]{
            auto msg_map = std::unordered_map<AxisFlag, std::string>{
                    {AxisFlag::Height, "<span style=\" font-weight:600; color:#ff0000;\">Height</span> should be greater than the thickness of top to bottom panels."},
                    {AxisFlag::Length, "<span style=\" font-weight:600; color:#ff0000;\">Length</span> should be greater than the thickness of left to right panels."},
                    {AxisFlag::Width, "<span style=\" font-weight:600; color:#ff0000;\">Width</span> should be greater than the thickness of front to back panels."},
            };

            auto av = this->m_axis_list[axis_flag];
            auto dimension_control = adsk::core::Ptr<FloatSpinnerCommandInput>{this->m_controls[dimension_id]};

            auto sum_thicknesses = [](double x, const EnableThicknessPair& input) {
                auto is_enabled = input.enable->value();
                auto value = input.thickness->value();
                return (is_enabled ? value : 0) + x;
            };

            auto minimum = std::accumulate (av.begin(), av.end(), 0.0, sum_thicknesses);

            auto value = dimension_control->value();
            auto result = value > minimum;

            this->m_error_message = msg_map[axis_flag];

            return result;
        };
        m_validators.emplace_back(validator);
    }
}

void CreateDialog::addMaximumKerfCheck() {
    auto kerf = adsk::core::Ptr<FloatSpinnerCommandInput>{m_controls[DialogInputs::Kerf]};
    auto validator = [this, kerf](){
        if (this->m_thicknesses.size() < 2) {
            this->m_error_message = "A minimum of two panels must be selected.";
            return false;
        }

        auto element = *std::min_element(this->m_thicknesses.begin(), this->m_thicknesses.end(), [](
            const EnableThicknessControl& lhs, const EnableThicknessControl& rhs){
                auto lhc = (lhs.is_enabled->value() && lhs.is_overridden->value()) ? lhs.use_override : lhs.use_default;
                auto rhc = (rhs.is_enabled->value() && rhs.is_overridden->value()) ? rhs.use_override : rhs.use_default;
                return lhc->value() < rhc->value();
            });

        auto control = (element.is_enabled->value() && element.is_overridden->value()) ? element.use_override : element.use_default;

        if (kerf->value() < control->value()) return true;

        this->m_error_message = "<span style=\" font-weight:600; color:#ff0000;\">Kerf</span> is larger than the smallest panel thickness.";

        return false;
    };
    m_validators.emplace_back(validator);
}

void CreateDialog::addMinimumFingerWidthCheck() {
    auto finger_width = adsk::core::Ptr<FloatSpinnerCommandInput>{m_controls[DialogInputs::FingerWidth]};

    auto validator = [this, finger_width](){
        if (this->m_thicknesses.size() < 2) {
            this->m_error_message = "A minimum of two panels must be selected.";
            return false;
        }

        auto element = *std::max_element(this->m_thicknesses.begin(), this->m_thicknesses.end(), [](
            const EnableThicknessControl& lhs, const EnableThicknessControl& rhs){
            auto lhc = (lhs.is_enabled->value() && lhs.is_overridden->value()) ? lhs.use_override : lhs.use_default;
            auto rhc = (rhs.is_enabled->value() && rhs.is_overridden->value()) ? rhs.use_override : rhs.use_default;
            return lhc->value() < rhc->value();
        });

        auto control = (element.is_enabled->value() && element.is_overridden->value()) ? element.use_override : element.use_default;

        if (finger_width->value() > control->value()) return true;

        this->m_error_message = "<span style=\" font-weight:600; color:#ff0000;\">Finger width</span> should be larger than the largest panel thickness.";

        return false;
    };
    m_validators.emplace_back(validator);
}
