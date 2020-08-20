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
#include "entities/PanelDimensionInputs.hpp"
#include "entities/PanelId.hpp"
#include "entities/PanelJoints.hpp"
#include "entities/PanelName.hpp"
#include "entities/PanelOffsetInput.hpp"
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

#include "plog/Log.h"

using std::accumulate;
using std::all_of;
using std::get;
using std::vector;

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox;

void CreateDialog::clear() {
    m_validators.clear();
    m_handlers.clear();
    m_results.clear();
    m_inputs.clear();
    m_ignore_updates.clear();
    m_configuration.clear();
}

void CreateDialog::create(const adsk::core::Ptr<Application> &app, const adsk::core::Ptr<CommandInputs> &inputs, const adsk::core::Ptr<Component> &root,
                          DefaultModelingOrientations orientation,
                          bool is_metric) {
    m_app = app;
    m_configuration.set<DialogBoxModelingUnits>(is_metric);
    m_configuration.set<DialogBoxCommandInputs>(inputs);
    m_configuration.set<DialogBoxApplication>(app);
    m_configuration.set<DialogBoxRootComponent>(root);
    m_configuration.set<DialogBoxModelingOrientation>(orientation);

    auto const& dimensions = inputs->addTabCommandInput("dimensionsTabInput", "", "resources/dimensions");
    dimensions->tooltip("Dimensions");

    auto dimension_children = dimensions->children();

    createModelSelectionDropDown(dimension_children);
    createFingerModeSelectionDropDown(dimension_children);

    auto dimensions_group = createDimensionGroup(dimension_children);
    createPanelTable(dimensions_group->children());

    auto const& dividers = inputs->addTabCommandInput("dividersTabInput", "", "resources/dividers");
    dividers->tooltip("Dividers");
    createDividerInputs(dividers->children());

    auto const& insets = inputs->addTabCommandInput("insertPanelsTabInput", "Insets");
    insets->tooltip("Panel Insets");
    createOffsetInputs(insets->children());

    createPreviewTable(inputs);
    m_error = inputs->addTextBoxCommandInput("errorMessageCommandInput", "", "", 2, true);
    m_error->isVisible(false);

    m_ignore_updates.emplace_back(m_error->id());

    addMinimumAxisDimensionChecks();
    addMaximumKerfCheck();
    addMinimumFingerWidthCheck();
    addMinimumPanelCountCheck();

    updatePanelOrientations();
    createPanelJoints();
}

void CreateDialog::createModelSelectionDropDown(const Ptr<CommandInputs>& inputs) {
    auto creation_mode = inputs->addDropDownCommandInput("creationTypeCommandInput", "Type of Model", TextListDropDownStyle);
    m_configuration.set<DialogBoxCreationMode>(creation_mode);

    auto const& creation_items = creation_mode->listItems();
    creation_items->add("Parametric", true);
    creation_items->add("Direct Model", false);
    creation_mode->maxVisibleItems(2);

    addInputControl(DialogInputs::ModelSelection, creation_mode, [this, creation_mode](){
        auto const& full = m_configuration.ctx<DialogFullPreviewMode>().control;
        auto const& label = m_configuration.ctx<DialogFullPreviewLabel>().control;

        if (full->value()) {
            full->value(false);
        }

        auto const is_parametric_mode = creation_mode->selectedItem()->index() == 0;
        full->isVisible(is_parametric_mode);
        label->isVisible(is_parametric_mode);
    });
}

void CreateDialog::createFingerModeSelectionDropDown(const Ptr<CommandInputs>& inputs) {
    auto finger_mode = inputs->addDropDownCommandInput("fingerTypeCommandInput", "Finger Size", TextListDropDownStyle);
    m_configuration.set<DialogBoxFingerMode>(finger_mode);

    auto const& creation_items = finger_mode->listItems();
    creation_items->add("Automatic Width", true);
    creation_items->add("Constant Width", false);
    creation_items->add("None", false);
    finger_mode->maxVisibleItems(3);

    addInputControl(DialogInputs::FingerMode, finger_mode);
}

void CreateDialog::createDividerInputs(const Ptr<CommandInputs>& inputs) {
    auto divider_joint = inputs->addDropDownCommandInput("dividerLapCommandInput", "Top Joint", TextListDropDownStyle);
    m_configuration.set<DialogBoxDividerJointInput>(divider_joint);

    auto const& joint_items = divider_joint->listItems();
    joint_items->add("Length Dividers", false);
    joint_items->add("Width Dividers", true);
    divider_joint->maxVisibleItems(2);

    addInputControl(DialogInputs::DividerLapInput, divider_joint, [this](){
        update(m_configuration.ctx<DialogBoxLengthDividerCountInput>().control);
        update(m_configuration.ctx<DialogBoxWidthDividerCountInput>().control);
    });

    auto length_divider_outside_joint = inputs->addDropDownCommandInput("lengthDividerOutsideJointInput", "Length Divider Joint", TextListDropDownStyle);
    m_configuration.set<DialogBoxLengthDividerJointInput>(length_divider_outside_joint);

    auto const& length_items = length_divider_outside_joint->listItems();
    length_items->add("Tenon", true);
    length_items->add("Half Lap", false);
    length_items->add("Box Joint", false);
    length_divider_outside_joint->maxVisibleItems(3);

    addInputControl(DialogInputs::LengthDividerJointInput, length_divider_outside_joint, [this](){
        update(m_configuration.ctx<DialogBoxLengthDividerCountInput>().control);
    });

    m_panel_registry.view<JointLengthOrientation, InsideJointPattern, OutsidePanel>().each([this](
        auto entity, auto const& orientation, auto const& pattern_position, auto const& panel_position
    ){
        auto const& length_divider_outside_joint = m_configuration.ctx<DialogBoxLengthDividerJointInput>().control;
        m_panel_registry.emplace<LengthJointInput>(entity, length_divider_outside_joint);
    });

    auto length = adsk::core::Ptr<IntegerSpinnerCommandInput>{inputs->addIntegerSpinnerCommandInput(
        "lengthDividerCommandInput", "(#) Length Dividers", 0, 25, 1, 0
    )};
    m_configuration.set<DialogBoxLengthDividerCountInput>(length);

    addInputControl(DialogInputs::LengthDividerCount, length, [this](){
        auto const& divider_joint = m_configuration.ctx<DialogBoxDividerJointInput>().control;
        auto const& length_divider_outside_joint = m_configuration.ctx<DialogBoxLengthDividerJointInput>().control;
        auto const divider_count = m_configuration.ctx<DialogBoxLengthDividerCountInput>().control->value();
        auto const divider_length = m_configuration.ctx<DialogBoxLengthInput>().control->value();

        auto old_view = m_panel_registry.view<InsidePanel, LengthOrientation>();
        m_panel_registry.destroy(old_view.begin(), old_view.end());

        auto const outside_joint_type = length_divider_outside_joint->selectedItem()->index();
        auto const inside_joint_type = divider_joint->selectedItem()->index();

        auto joint_selector = std::map<int, JointType>{
            {0, JointType::TopLap},
            {1, JointType::BottomLap}
        };
        auto joints = addInsideJoints(AxisFlag::Length, joint_selector[inside_joint_type], outside_joint_type);

        auto dividers = Dividers<LengthOrientation>(m_panel_registry, m_configuration, m_app, joints);
        dividers.create("Length", divider_count, divider_length, AxisFlag::Length);
    });

    auto width_divider_outside_joint = inputs->addDropDownCommandInput("widthDividerOutsideJointInput", "Width Divider Joint", TextListDropDownStyle);
    m_configuration.set<DialogBoxWidthDividerJointInput>(width_divider_outside_joint);

    auto const& width_items = width_divider_outside_joint->listItems();
    width_items->add("Tenon", true);
    width_items->add("Half Lap", false);
    width_items->add("Box Joint", false);
    width_divider_outside_joint->maxVisibleItems(3);

    addInputControl(DialogInputs::WidthDividerJointInput, width_divider_outside_joint, [this](){
        update(m_configuration.ctx<DialogBoxWidthDividerCountInput>().control);
    });

    m_panel_registry.view<JointWidthOrientation, InsideJointPattern, OutsidePanel>().each([this](
        auto entity, auto const& orientation, auto const& pattern_position, auto const& panel_position
    ){
        auto const& width_divider_outside_joint = m_configuration.ctx<DialogBoxWidthDividerJointInput>().control;
        m_panel_registry.emplace<WidthJointInput>(entity, width_divider_outside_joint);
    });

    auto width = adsk::core::Ptr<IntegerSpinnerCommandInput>{inputs->addIntegerSpinnerCommandInput(
        "widthDividerCommandInput", "(#) Width Dividers", 0, 25, 1, 0
    )};
    m_configuration.set<DialogBoxWidthDividerCountInput>(width);

    addInputControl(DialogInputs::WidthDividerCount, width, [this](){
        auto const& divider_joint = m_configuration.ctx<DialogBoxDividerJointInput>().control;
        auto const& width_divider_outside_joint = m_configuration.ctx<DialogBoxWidthDividerJointInput>().control;
        auto const divider_count = m_configuration.ctx<DialogBoxWidthDividerCountInput>().control->value();
        auto const divider_length = m_configuration.ctx<DialogBoxWidthInput>().control->value();

        auto const outside_joint_type = width_divider_outside_joint->selectedItem()->index();
        auto const inside_joint_type = divider_joint->selectedItem()->index();

        auto old_view = m_panel_registry.view<InsidePanel, WidthOrientation>();
        m_panel_registry.destroy(old_view.begin(), old_view.end());

        auto joint_selector = std::map<int, JointType>{
            {0, JointType::BottomLap},
            {1, JointType::TopLap}
        };
        auto joints = addInsideJoints(AxisFlag::Width, joint_selector[inside_joint_type], outside_joint_type);

        auto dividers = Dividers<WidthOrientation>(m_panel_registry, m_configuration, m_app, joints);
        dividers.create("Width", divider_count, divider_length, AxisFlag::Width);
    });

//    auto height = adsk::core::Ptr<IntegerSpinnerCommandInput>{inputs->addIntegerSpinnerCommandInput(
//        "heightDividerCommandInput", "(#) Height Dividers", 0, 25, 1, 0
//    )};
//    m_panel_registry.view<JointHeightOrientation>().each([this](
//        auto entity
//    ){
//        m_panel_registry.emplace<HeightJointInput>(entity);
//    });
//
//    addInputControl(DialogInputs::HeightDividerCount, height, [this](){
//
//        auto old_view = m_panel_registry.view<InsidePanel, HeightOrientation>();
//        m_panel_registry.destroy(old_view.begin(), old_view.end());
//
//        auto dividers = Dividers<HeightOrientation>(m_panel_registry, m_app, m_controls);
//        dividers.create("Height", DialogInputs::HeightDividerCount, DialogInputs::Height, AxisFlag::Height);
//    });

    addInputHandler(DialogInputs::Length, [this](){
        update(m_configuration.ctx<DialogBoxLengthDividerCountInput>().control);
        update(m_configuration.ctx<DialogBoxWidthDividerCountInput>().control);
    });

    addInputHandler(DialogInputs::Width, [this](){
        update(m_configuration.ctx<DialogBoxLengthDividerCountInput>().control);
        update(m_configuration.ctx<DialogBoxWidthDividerCountInput>().control);
    });

    addInputHandler(DialogInputs::Height, [this](){
        update(m_configuration.ctx<DialogBoxLengthDividerCountInput>().control);
        update(m_configuration.ctx<DialogBoxWidthDividerCountInput>().control);
    });

}

void CreateDialog::createOffsetInputs(const Ptr<CommandInputs>& inputs) {
    auto const inset_panels = inputs->addDropDownCommandInput("insetPanelsCommandInput", "Inset Panels", TextListDropDownStyle);
    m_configuration.set<DialogBoxInsetPanelsInput>(inset_panels);

    auto const& joint_items = inset_panels->listItems();
    joint_items->add("Top", true);
    joint_items->add("Bottom", false);
    joint_items->add("Front", false);
    joint_items->add("Back", false);
    joint_items->add("Left", false);
    joint_items->add("Right", false);
    joint_items->add("Top/Bottom", false);
    joint_items->add("Front/Back", false);
    joint_items->add("Left/Right", false);
    inset_panels->maxVisibleItems(9);

    auto const bottom_offset = inputs->addFloatSpinnerCommandInput(
        "bottomPanelOffsetCommandInput", "Bottom Panel Offset", "mm", 0, 2540, 1, 0
    );
    m_panel_registry.set<PanelOffsetInput>(bottom_offset);
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

    auto const fast_preview = table->commandInputs()->addBoolValueInput("fastPreviewCommandInput", "Fast Preview", true, "", false);
    auto const fast_label = table->commandInputs()->addTextBoxCommandInput("fastPreviewLabelInput", "Fast Preview", "Fast Preview", 1, true);
    auto const full_preview = table->commandInputs()->addBoolValueInput("fullPreviewCommandInput", "Full Preview", true, "", false);
    auto const full_label = table->commandInputs()->addTextBoxCommandInput("fastPreviewLabelInput", "Full Preview", "Full Preview", 1, true);

    m_configuration.set<DialogFastPreviewMode>(fast_preview);
    m_configuration.set<DialogFastPreviewLabel>(fast_label);
    m_configuration.set<DialogFullPreviewMode>(full_preview);
    m_configuration.set<DialogFullPreviewLabel>(full_label);

    table->addCommandInput(fast_preview, 0, 0);
    table->addCommandInput(fast_label, 0, 1);
    table->addCommandInput(full_preview, 0, 2);
    table->addCommandInput(full_label, 0, 3);

    addInputControl(DialogInputs::FastPreviewLabel, fast_label);
    addInputControl(DialogInputs::FastPreview, fast_preview, [this](){
        auto const& fast_preview = m_configuration.ctx<DialogFastPreviewMode>().control;
        auto const& full_preview = m_configuration.ctx<DialogFullPreviewMode>().control;

        if (fast_preview->value()) {
            full_preview->value(false);
        }
    });

    addInputControl(DialogInputs::FullPreviewLabel, full_label);
    addInputControl(DialogInputs::FullPreview, full_preview, [this](){
        auto const& fast_preview = m_configuration.ctx<DialogFastPreviewMode>().control;
        auto const& full_preview = m_configuration.ctx<DialogFullPreviewMode>().control;

        if (full_preview->value()) {
            fast_preview->value(false);
        }
    });
}

void CreateDialog::addInputControl(const DialogInputs reference, const adsk::core::Ptr<CommandInput>& input) {
    m_inputs[input->id()] = reference;
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

auto CreateDialog::createDimensionGroup(const adsk::core::Ptr<CommandInputs>& inputs) -> adsk::core::Ptr<GroupCommandInput> {
    auto group = inputs->addGroupCommandInput("dimensionsGroupInput", "Dimensions");
    auto is_metric = m_configuration.ctx<DialogBoxModelingUnits>().value;
    auto children = group->children();

    m_configuration.set<DialogBoxLengthInput>(createDimensionInput(children, length_defaults[is_metric]));
    m_configuration.set<DialogBoxWidthInput>(createDimensionInput(children, width_defaults[is_metric]));
    m_configuration.set<DialogBoxHeightInput>(createDimensionInput(children, height_defaults[is_metric]));
    m_configuration.set<DialogBoxThicknessInput>(createDimensionInput(children, thickness_defaults[is_metric]));
    m_configuration.set<DialogBoxFingerWidthInput>(createDimensionInput(children, finger_defaults[is_metric]));
    m_configuration.set<DialogBoxKerfInput>(createDimensionInput(children, kerf_defaults[is_metric]));

    return group;
}

auto CreateDialog::createDimensionInput(Ptr<CommandInputs> &children, const InputConfig& config) -> Ptr<FloatSpinnerCommandInput> {

    auto spinner = Ptr<FloatSpinnerCommandInput>{children->addFloatSpinnerCommandInput(
            config.id, config.name, config.unit_type, config.minimum,
            config.maximum, config.step, config.initial_value
    )};

    auto validator = [spinner]{
        return spinner->value() >= spinner->minimumValue();
    };
    m_validators.emplace_back(validator);

    addInputControl(config.lookup, spinner);

    return spinner;
}

void CreateDialog::createPanelTable(
    const Ptr<CommandInputs>& inputs
) {
    auto table = initializePanelTable(inputs);

    auto top = addPanelTableRow<DialogBoxTopInputs, DialogBoxTopThickness>(inputs, table, m_top_row);
    m_configuration.set<DialogBoxTopPanel>(top);

    auto bottom = addPanelTableRow<DialogBoxBottomInputs, DialogBoxBottomThickness>(inputs, table, m_bottom_row);
    m_configuration.set<DialogBoxBottomPanel>(bottom);

    auto left = addPanelTableRow<DialogBoxLeftInputs, DialogBoxLeftThickness>(inputs, table, m_left_row);
    m_configuration.set<DialogBoxLeftPanel>(left);

    auto right = addPanelTableRow<DialogBoxRightInputs, DialogBoxRightThickness>(inputs, table, m_right_row);
    m_configuration.set<DialogBoxRightPanel>(right);

    auto front = addPanelTableRow<DialogBoxFrontInputs, DialogBoxFrontThickness>(inputs, table, m_front_row);
    m_configuration.set<DialogBoxFrontPanel>(front);

    auto back = addPanelTableRow<DialogBoxBackInputs, DialogBoxBackThickness>(inputs, table, m_back_row);
    m_configuration.set<DialogBoxBackPanel>(back);

}

template <class T, class U>
auto CreateDialog::addPanelTableRow(
    const Ptr<CommandInputs> &inputs,
    Ptr<adsk::core::TableCommandInput> &table,
    const DimensionTableRow &row
) -> entt::entity {
    auto entity = m_configuration.create();

    auto is_metric         = m_configuration.ctx<DialogBoxModelingUnits>().value;
    auto default_thickness = m_configuration.ctx<DialogBoxThicknessInput>().control;
    auto row_num           = table->rowCount();

    auto label_control    = addPanelLabelControl(inputs, row);
    auto enable_control   = addPanelEnableControl(inputs, row);
    auto override_control = addPanelOverrideControl(inputs, row);
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

    m_configuration.emplace<DialogBoxPanelEnable>(entity, enable_control);
    m_configuration.emplace<DialogBoxPanelInputs>(entity, label_control, enable_control, override_control, thickness_control);
    m_configuration.emplace<DialogBoxPanelLabel>(entity, label_control);
    m_configuration.emplace<DialogBoxPanelOverride>(entity, override_control);
    m_configuration.emplace<DialogBoxPanelOverrideThickness>(entity, thickness_control);
    m_configuration.emplace<DialogBoxPanelThickness>(entity, default_thickness);
    m_configuration.emplace<OutsidePanel>(entity);
    m_configuration.emplace<PanelOrientation>(entity, row.orientation);
    m_configuration.emplace<PanelPosition>(entity, Position::Outside);
    m_configuration.emplace<PanelName>(entity, row.label.name);

    m_configuration.set<T>(label_control, enable_control, override_control, thickness_control);
    m_configuration.set<U>(thickness_control);

    auto thickness_swap = [this, entity, override_control, thickness_control]{
        auto const default_thickness = m_configuration.ctx<DialogBoxThicknessInput>().control;

        auto t_control = Ptr<CommandInput>{thickness_control};
        auto d_control = Ptr<CommandInput>{default_thickness};

        auto toggle = override_control->value() ? t_control : d_control;

        m_configuration.replace<DialogBoxPanelThickness>(entity, toggle);
        m_configuration.set<U>(toggle);
    };

    auto thickness_toggle = [override_control, thickness_control]{
        thickness_control->isEnabled(override_control->value() && override_control->isEnabled());
    };

    auto override_toggle = [override_control, enable_control]{
        override_control->isEnabled(enable_control->value());
    };

    auto follow_thickness = [this, override_control, thickness_control]{
        auto const& default_thickness = m_configuration.ctx<DialogBoxThicknessInput>().control;

        auto thickness_enabled = override_control->value();

        if (thickness_enabled)
            return;

        thickness_control->value(default_thickness->value());
    };

    addInputControl(row.override.lookup, override_control, {thickness_swap, thickness_toggle, follow_thickness});
    addInputControl(row.enable.lookup, enable_control, {thickness_toggle, override_toggle});
    addInputControl(row.thickness.lookup, thickness_control);
    addInputHandler(DialogInputs::Thickness, follow_thickness);

    return entity;
}

auto CreateDialog::addPanelOverrideControl(const Ptr<CommandInputs> &inputs, const DimensionTableRow &row) -> Ptr<BoolValueCommandInput>{
    auto override_control = inputs->addBoolValueInput(
        row.override.id, row.override.name, true, "", false
    );
    return override_control;
}

auto CreateDialog::addPanelEnableControl(const Ptr<CommandInputs> &inputs, const DimensionTableRow &row) -> Ptr<BoolValueCommandInput>{
    auto enable_control = inputs->addBoolValueInput(
        row.enable.id, row.enable.name, true, "", row.enable.default_value
    );
    return enable_control;
}

auto CreateDialog::addPanelLabelControl(const Ptr<CommandInputs> &inputs, const DimensionTableRow &row) -> Ptr<TextBoxCommandInput>{
    auto label_control = inputs->addTextBoxCommandInput(
        row.label.id, row.label.name, "<b>" + row.label.name + "</b>", 1, true
    );
    return label_control;
}

auto CreateDialog::initializePanelTable(const adsk::core::Ptr<CommandInputs> &inputs) const -> adsk::core::Ptr<TableCommandInput> {
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

template<class T, class U>
bool validateDimension(entt::registry& m_configuration, AxisFlag axis_flag) {
    auto msg_map = std::unordered_map<AxisFlag, std::string>{
        {AxisFlag::Height, "<span style=\" font-weight:600; color:#ff0000;\">Height</span> should be greater than the thickness of top to bottom panels."},
        {AxisFlag::Length, "<span style=\" font-weight:600; color:#ff0000;\">Length</span> should be greater than the thickness of left to right panels."},
        {AxisFlag::Width, "<span style=\" font-weight:600; color:#ff0000;\">Width</span> should be greater than the thickness of front to back panels."},
    };

    auto dimension_control = m_configuration.ctx<T>().control;

    auto minimum = 0.0;

    m_configuration.view<U, DialogBoxPanelEnable, DialogBoxPanelThickness>().each([&](
        auto entity, auto const& dimension, auto const& enable, auto const& thickness
    ){
        minimum += thickness.control->value() * enable.control->value();
    });

    auto value = dimension_control->value();
    if (value > minimum) return true;

    m_configuration.set<DialogBoxErrorMessage>(msg_map[axis_flag]);

    return false;
}

void CreateDialog::addMinimumAxisDimensionChecks() {
    auto validator = [this]() {
        auto length_ok = validateDimension<DialogBoxLengthInput, LengthOrientation>(m_configuration, AxisFlag::Length);
        auto width_ok = validateDimension<DialogBoxWidthInput, WidthOrientation>(m_configuration, AxisFlag::Width);
        auto height_ok = validateDimension<DialogBoxHeightInput, HeightOrientation>(m_configuration, AxisFlag::Height);

        return length_ok && width_ok && height_ok;
    };
    m_validators.emplace_back(validator);
}

void CreateDialog::addMaximumKerfCheck() {
    auto validator = [this](){
        auto kerf = m_configuration.ctx<DialogBoxKerfInput>().control->value();

        bool kerf_ok = true;

        m_configuration.view<DialogBoxPanelThickness>().each([&](
            auto const& thickness
        ){
           kerf_ok = kerf_ok && (thickness.control->value() > kerf);
        });

        if (kerf_ok) return true;

        m_configuration.set<DialogBoxErrorMessage>(
            "<span style=\" font-weight:600; color:#ff0000;\">Kerf</span> is larger than the smallest panel thickness."
        );

        return false;
    };
    m_validators.emplace_back(validator);
}

void CreateDialog::addMinimumFingerWidthCheck() {
    auto validator = [this](){
        auto finger_width = m_configuration.ctx<DialogBoxFingerWidthInput>().control->value();

        bool finger_ok = true;

        m_configuration.view<DialogBoxPanelThickness>().each([&](
            auto const& thickness
        ){
            finger_ok = finger_ok && (thickness.control->value() < finger_width);
        });

        if (finger_ok) return true;

        m_configuration.set<DialogBoxErrorMessage>(
            "<span style=\" font-weight:600; color:#ff0000;\">Finger width</span> should be larger than the largest panel thickness."
        );

        return false;
    };
    m_validators.emplace_back(validator);
}

void CreateDialog::addMinimumPanelCountCheck() {
    auto validator = [this](){
        int thickness_count = 0;

        m_configuration.view<DialogBoxPanelEnable>().each([&](
            auto const& enable
        ){
           thickness_count += enable.control->value();
        });

        if (thickness_count < 2) {
            m_configuration.set<DialogBoxErrorMessage>(
                "<span style=\" font-weight:600; color:#ff0000;\">A minimum of two panels must be selected."
            );
            return false;
        }

        return true;
    };
    m_validators.emplace_back(validator);
}

bool CreateDialog::validate(const adsk::core::Ptr<CommandInputs>& inputs) {
    auto m_error_message = m_configuration.ctx<DialogBoxErrorMessage>().value;
    m_error->formattedText(m_error_message);

    auto results = all_of(m_validators.begin(), m_validators.end(), [](const std::function<bool()>& v){ return v(); });

    m_error->isVisible(!results);

    return results;
}

bool CreateDialog::update(const adsk::core::Ptr<CommandInput>& cmd_input) {
    auto exists = std::find(m_ignore_updates.begin(), m_ignore_updates.end(), cmd_input->id()) != m_ignore_updates.end();
    if (exists) return false;

    auto handlers = m_handlers[m_inputs[cmd_input->id()]];

    for (auto &handler: handlers) {
        handler();
    }
    return true;
}

void CreateDialog::updatePanelOrientations() {
    addPanelOrientation<LengthOrientation, AxisFlag::Length>(m_configuration);
    addPanelOrientation<WidthOrientation, AxisFlag::Width>(m_configuration);
    addPanelOrientation<HeightOrientation, AxisFlag::Height>(m_configuration);
}


template <class L, class W, class H>
void CreateDialog::addPanelDimensions(entt::entity entity) {
    auto length = m_configuration.ctx<L>().control;
    auto width = m_configuration.ctx<W>().control;
    auto height = m_configuration.ctx<H>().control;

    m_configuration.emplace<PanelDimensionInputs>(entity, length, width, height);
}

template <class M>
void CreateDialog::addMaxOffset(entt::entity entity) {
    auto offset = m_configuration.ctx<M>().control;

    m_configuration.emplace<MaxOffsetInput>(entity, offset);
}

template <class M>
void CreateDialog::addPanelJoints(entt::entity entity, entt::entity first, entt::entity second) {
    m_configuration.emplace<PanelJoints>(entity, PanelJoints{{first, second}});
}

template <class T, class J>
auto CreateDialog::addPanelJoint() -> entt::entity{

    auto kerf = m_configuration.ctx<DialogBoxKerfInput>().control;
    auto finger_width = m_configuration.ctx<DialogBoxFingerWidthInput>().control;
    auto finger_mode = m_configuration.ctx<DialogBoxFingerMode>().control;
    auto panel_controls = m_configuration.ctx<T>();

    auto panel = m_panel_registry.create();

    m_panel_registry.emplace<OutsidePanel>(panel);
    m_panel_registry.emplace<KerfInput>(panel, kerf);
    m_panel_registry.emplace<FingerPatternInput>(panel, finger_mode);
    m_panel_registry.emplace<FingerWidthInput>(panel, finger_width);
    m_panel_registry.emplace<FingerPatternType>(panel, FingerMode::Automatic);

    m_panel_registry.emplace<EnableInput>(panel, panel_controls.enabled);
    m_panel_registry.emplace<OverrideInput>(panel, panel_controls.override);
    m_panel_registry.emplace<ThicknessInput>(panel, panel_controls.thickness);

    m_panel_registry.emplace<Dimensions>(panel);
    m_panel_registry.emplace<EndReferencePoint>(panel);
    m_panel_registry.emplace<ExtrusionDistance>(panel);
    m_panel_registry.emplace<FingerWidth>(panel);
    m_panel_registry.emplace<JointPanelOffset>(panel);
    m_panel_registry.emplace<JointPatternDistance>(panel);
    m_panel_registry.emplace<JointThickness>(panel);
    m_panel_registry.emplace<MaxOffset>(panel);
    m_panel_registry.emplace<PanelOffset>(panel);
    m_panel_registry.emplace<PanelProfile>(panel);
    m_panel_registry.emplace<StartReferencePoint>(panel);

    m_panel_registry.emplace<J>(panel);

    return panel;
}

void CreateDialog::createPanelJoints() {
    auto orientation = m_configuration.ctx<DialogBoxModelingOrientation>();

    auto top = m_configuration.ctx<DialogBoxTopPanel>().id;
    auto bottom = m_configuration.ctx<DialogBoxBottomPanel>().id;
    auto left = m_configuration.ctx<DialogBoxLeftPanel>().id;
    auto right = m_configuration.ctx<DialogBoxRightPanel>().id;
    auto front = m_configuration.ctx<DialogBoxFrontPanel>().id;
    auto back = m_configuration.ctx<DialogBoxBackPanel>().id;

    addPanelDimensions<DialogBoxLengthInput, DialogBoxWidthInput, DialogBoxHeightInput>(top);
    addMaxOffset<DialogBoxHeightInput>(top);
    auto top_length = addPanelJoint<DialogBoxTopInputs, JointLengthOrientation>();
    auto top_width = addPanelJoint<DialogBoxTopInputs, JointWidthOrientation>();
    addPanelJoints<DialogBoxHeightInput>(top, top_length, top_width);

    addPanelDimensions<DialogBoxLengthInput, DialogBoxWidthInput, DialogBoxBottomThickness>(bottom);
    addMaxOffset<DialogBoxHeightInput>(bottom);
    auto bottom_length = addPanelJoint<DialogBoxBottomInputs, JointLengthOrientation>();
    auto bottom_width = addPanelJoint<DialogBoxBottomInputs, JointWidthOrientation>();
    addPanelJoints<DialogBoxHeightInput>(bottom, bottom_length, bottom_width);

    addPanelDimensions<DialogBoxLeftThickness, DialogBoxWidthInput, DialogBoxHeightInput>(left);
    addMaxOffset<DialogBoxLengthInput>(left);
    auto left_length = addPanelJoint<DialogBoxLeftInputs, JointHeightOrientation>();
    auto left_width = addPanelJoint<DialogBoxLeftInputs, JointWidthOrientation>();
    addPanelJoints<DialogBoxLengthInput>(left, left_length, left_width);

    addPanelDimensions<DialogBoxLengthInput, DialogBoxWidthInput, DialogBoxHeightInput>(right);
    addMaxOffset<DialogBoxLengthInput>(right);
    auto right_length = addPanelJoint<DialogBoxRightInputs, JointHeightOrientation>();
    auto right_width = addPanelJoint<DialogBoxRightInputs, JointWidthOrientation>();
    addPanelJoints<DialogBoxLengthInput>(right, right_length, right_width);

    if (orientation.value == YUpModelingOrientation) {
        addPanelDimensions<DialogBoxLengthInput, DialogBoxWidthInput, DialogBoxHeightInput>(front);
        addPanelDimensions<DialogBoxLengthInput, DialogBoxBackThickness, DialogBoxHeightInput>(back);
    } else {
        addPanelDimensions<DialogBoxLengthInput, DialogBoxFrontThickness, DialogBoxHeightInput>(front);
        addPanelDimensions<DialogBoxLengthInput, DialogBoxWidthInput, DialogBoxHeightInput>(back);
    }

    addMaxOffset<DialogBoxWidthInput>(front);
    auto front_length = addPanelJoint<DialogBoxFrontInputs, JointHeightOrientation>();
    auto front_width = addPanelJoint<DialogBoxFrontInputs, JointLengthOrientation>();
    addPanelJoints<DialogBoxWidthInput>(front, front_length, front_width);

    addMaxOffset<DialogBoxWidthInput>(back);
    auto back_length = addPanelJoint<DialogBoxBackInputs, JointHeightOrientation>();
    auto back_width = addPanelJoint<DialogBoxBackInputs, JointLengthOrientation>();
    addPanelJoints<DialogBoxWidthInput>(back, back_length, back_width);

    m_configuration.view<PanelDimensionInputs, PanelJoints>().each([this](
        auto const& dimensions, auto const& panel
    ){
        for (auto const& joint: panel.joints) {
            m_panel_registry.emplace<DimensionsInputs>(joint, dimensions.length, dimensions.width, dimensions.height);
        }
    });

    m_configuration.view<PanelOrientation, PanelName, PanelPosition, PanelJoints>().each([this](
        auto const& orientation, auto const& name, auto const& position, auto const& panel
    ){
        for (auto const& joint: panel.joints) {
            m_panel_registry.emplace<PanelOrientation>(joint, orientation);
            m_panel_registry.emplace<PanelName>(joint, name);
            m_panel_registry.emplace<PanelPosition>(joint, position);
        }
    });

    m_configuration.view<LengthOrientation, PanelJoints>().each([this](
        auto const& orientation, auto const& panel
    ){
        for (auto const& joint: panel.joints) {
            m_panel_registry.emplace<LengthOrientation>(joint);
        }
    });

    m_configuration.view<WidthOrientation, PanelJoints>().each([this](
        auto const& orientation, auto const& panel
    ){
        for (auto const& joint: panel.joints) {
            m_panel_registry.emplace<WidthOrientation>(joint);
        }
    });

    m_configuration.view<HeightOrientation, PanelJoints>().each([this](
        auto const& orientation, auto const& panel
    ){
        for (auto const& joint: panel.joints) {
            m_panel_registry.emplace<HeightOrientation>(joint);
        }
    });

    m_panel_registry.view<JointHeightOrientation>().each([this](
        auto entity, auto const& orientation
    ){
        m_panel_registry.emplace<JointOrientation>(entity, AxisFlag::Height);
    });

    m_panel_registry.view<JointLengthOrientation>().each([this](
        auto entity, auto const& orientation
    ){
        m_panel_registry.emplace<JointOrientation>(entity, AxisFlag::Length);
    });

    m_panel_registry.view<JointWidthOrientation>().each([this](
        auto entity, auto const& orientation
    ){
        m_panel_registry.emplace<JointOrientation>(entity, AxisFlag::Width);
    });

    m_panel_registry.view<PanelPosition, PanelOrientation, JointOrientation>().each([this](
        auto entity, auto const& position, auto const& panel, auto const& joint
    ){
        m_panel_registry.emplace<OrientationGroup>(entity, panel.axis, joint.axis);
        m_panel_registry.emplace<JointProfile>(
            entity, position.value, Position::Outside, JointType::Normal, FingerMode::Automatic, 0, 0.0, 0.0, 0.0, 0.0, joint.axis, panel.axis
        ); // TODO Fix
        m_panel_registry.emplace<JointPatternPosition>(
            entity, Position::Outside, panel.axis, JointType::Normal, joint.axis, Position::Outside
        ); // TODO Fix
    });

}