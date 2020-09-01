//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/22/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//

#include "presentation/dialog.hpp"
#include "presentation/dividers.hpp"
#include "presentation/createdialogpanels.hpp"
#include "presentation/createpaneloverriderow.hpp"

#include "entities/AxisFlag.hpp"
#include "entities/ChildPanels.hpp"
#include "entities/Dimensions.hpp"
#include "entities/DividerTags.hpp"
#include "entities/Enabled.hpp"
#include "entities/EnableInput.hpp"
#include "entities/EndReferencePoint.hpp"
#include "entities/FingerWidth.hpp"
#include "entities/FingerWidthInput.hpp"
#include "entities/FingerPattern.hpp"
#include "entities/HeightJointInput.hpp"
#include "entities/InsidePanel.hpp"
#include "entities/JointEnabled.hpp"
#include "entities/JointDirection.hpp"
#include "entities/JointName.hpp"
#include "entities/JointOrientation.hpp"
#include "entities/JointPattern.hpp"
#include "entities/JointPatternDistance.hpp"
#include "entities/JointPatternTags.hpp"
#include "entities/JointDirection.hpp"
#include "entities/JointPosition.hpp"
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
#include "entities/PanelDimension.hpp"
#include "entities/PanelId.hpp"
#include "entities/PanelJoints.hpp"
#include "entities/Panel.hpp"
#include "entities/PanelOffsetInput.hpp"
#include "entities/PanelOrientation.hpp"
#include "entities/PanelPosition.hpp"
#include "entities/PanelProfile.hpp"
#include "entities/PanelType.hpp"
#include "entities/ParentPanel.hpp"
#include "entities/Position.hpp"
#include "entities/StandardJoint.hpp"
#include "entities/StartReferencePoint.hpp"
#include "entities/ToggleableThicknessInput.hpp"
#include "entities/Thickness.hpp"
#include "entities/WidthJointInput.hpp"

#include <numeric>

#include "plog/Log.h"
#include "FingerPattern.hpp"

using std::accumulate;
using std::all_of;
using std::get;
using std::vector;

//using namespace adsk::core;
//using namespace adsk::fusion;
using adsk::core::Ptr;
using adsk::core::Application;
using adsk::core::BoolValueCommandInput;
using adsk::core::CommandInputs;
using adsk::core::DefaultModelingOrientations;
using adsk::core::FloatSpinnerCommandInput;
using adsk::core::GroupCommandInput;
using adsk::core::IntegerSpinnerCommandInput;
using adsk::core::TabCommandInput;
using adsk::core::TableCommandInput;
using adsk::core::TablePresentationStyles;
using adsk::core::TablePresentationStyles::itemBorderTablePresentationStyle;
using adsk::core::TablePresentationStyles::transparentBackgroundTablePresentationStyle;
using adsk::core::TextListDropDownStyle;
using adsk::core::TextBoxCommandInput;
using adsk::fusion::Component;

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::dialog;

using silvanus::generatebox::entities::JointDirections;

void CreateDialog::clear() {
    m_validators.clear();
    m_handlers.clear();
    m_results.clear();
    m_inputs.clear();
    m_ignore_updates.clear();
    m_configuration.clear();
    m_panel_registry.clear();
}

void CreateDialog::create(
    const adsk::core::Ptr<Application> &app,
    const adsk::core::Ptr<CommandInputs> &inputs,
    const adsk::core::Ptr<Component> &root,
    DefaultModelingOrientations orientation,
    bool is_metric
) {
    m_app = app;
    m_configuration.set<DialogModelingUnits>(is_metric);
    m_configuration.set<DialogCommandInputs>(inputs);
    m_configuration.set<DialogApplication>(app);
    m_configuration.set<DialogRootComponent>(root);
    m_configuration.set<DialogModelingOrientation>(orientation);

    auto const &dimensions = inputs->addTabCommandInput("dimensionsTabInput", "", "resources/dimensions");
    dimensions->tooltip("Dimensions");

    auto dimension_children = dimensions->children();

    createModelSelectionDropDown(dimension_children);
    createFingerModeSelectionDropDown(dimension_children);

    auto dimensions_group = createDimensionGroup(dimension_children);
    createPanelTable(dimensions_group->children());

    auto const &dividers = inputs->addTabCommandInput("dividersTabInput", "", "resources/dividers");
    dividers->tooltip("Dividers");
    createDividerInputs(dividers->children());

//    auto const &insets = inputs->addTabCommandInput("insertPanelsTabInput", "Insets");
//    insets->tooltip("Panel Insets");
//    createOffsetInputs(insets->children());

    auto const &joints = inputs->addTabCommandInput("panelJointsTabInput", "Joints");
    joints->tooltip("Panel Joints");
    auto joint_table = createStandardJointTable(joints->children());

    createPreviewTable(inputs);
    m_error = inputs->addTextBoxCommandInput("errorMessageCommandInput", "", "", 2, true);
    m_error->isVisible(false);

    m_ignore_updates.emplace_back(m_error->id());

    addMinimumAxisDimensionChecks();
    addMaximumKerfCheck();
    addMinimumFingerWidthCheck();
    addMinimumPanelCountCheck();

    m_systems->initialize();
    m_systems->findJoints<OutsidePanel, OutsidePanel, StandardJoint>();
    m_systems->updateCollisions();

    populateJointTable(joint_table);

    m_systems->postUpdate();
    initializePanels();
}

void CreateDialog::addInputControl(const DialogInputs reference, const adsk::core::Ptr<CommandInput> &input) {
    m_inputs[reference] = input->id();
}

void CreateDialog::addInputControl(const DialogInputs reference, const adsk::core::Ptr<CommandInput> &input, const std::function<void()> &handler) {
    addInputControl(reference, input);
    m_handlers[input->id()].emplace_back(handler);
    m_inputs[reference] = input->id();
}

void CreateDialog::addInputControl(
    const DialogInputs reference, const adsk::core::Ptr<CommandInput> &input,
    const std::vector<std::function<void()>> &handlers
) {
    addInputControl(reference, input);
    for (auto const &handler: handlers) {
        m_handlers[input->id()].emplace_back(handler);
    }
}

void CreateDialog::addInputHandler(const DialogInputs reference, const std::function<void()> &handler) {
    m_handlers[m_inputs[reference]].emplace_back(handler);
}

void CreateDialog::addInputHandler(Ptr<CommandInput> &input, const std::function<void()> &handler) {
    m_handlers[input->id()].emplace_back(handler);
}

void CreateDialog::createModelSelectionDropDown(const Ptr<CommandInputs> &inputs) {
    auto creation_mode = inputs->addDropDownCommandInput("creationTypeCommandInput", "Type of Model", TextListDropDownStyle);
    m_configuration.set<DialogCreationMode>(creation_mode);

    auto const &creation_items = creation_mode->listItems();
    creation_items->add("Parametric", true);
    creation_items->add("Direct Model", false);
    creation_mode->maxVisibleItems(2);

    addInputControl(
        DialogInputs::ModelSelection, creation_mode, [this, creation_mode]() {
            auto const &full  = m_configuration.ctx<DialogFullPreviewMode>().control;
            auto const &label = m_configuration.ctx<DialogFullPreviewLabel>().control;

            if (full->value()) {
                full->value(false);
            }

            auto const is_parametric_mode = creation_mode->selectedItem()->index() == 0;
            full->isVisible(is_parametric_mode);
            label->isVisible(is_parametric_mode);
        }
    );
}

void CreateDialog::createFingerModeSelectionDropDown(const Ptr<CommandInputs> &inputs) {
    auto finger_mode = inputs->addDropDownCommandInput("fingerTypeCommandInput", "Finger Size", TextListDropDownStyle);
    m_configuration.set<DialogFingerMode>(finger_mode);

    auto const &creation_items = finger_mode->listItems();
    creation_items->add("Automatic Width", true);
    creation_items->add("Constant Width", false);
    creation_items->add("None", false);
    finger_mode->maxVisibleItems(3);

    addInputControl(DialogInputs::FingerMode, finger_mode);
}

auto CreateDialog::createDimensionGroup(const adsk::core::Ptr<CommandInputs> &inputs) -> adsk::core::Ptr<GroupCommandInput> {
    auto group     = inputs->addGroupCommandInput("dimensionsGroupInput", "Dimensions");
    auto is_metric = m_configuration.ctx<DialogModelingUnits>().value;
    auto children  = group->children();

    m_configuration.set<DialogLengthInput>(createDimensionInput(children, length_defaults[is_metric]));
    m_configuration.set<DialogWidthInput>(createDimensionInput(children, width_defaults[is_metric]));
    m_configuration.set<DialogHeightInput>(createDimensionInput(children, height_defaults[is_metric]));
    m_configuration.set<DialogThicknessInput>(createDimensionInput(children, thickness_defaults[is_metric]));
    m_configuration.set<DialogFingerWidthInput>(createDimensionInput(children, finger_defaults[is_metric]));
    m_configuration.set<DialogKerfInput>(createDimensionInput(children, kerf_defaults[is_metric]));

    return group;
}

auto CreateDialog::createDimensionInput(Ptr<CommandInputs> &children, const InputConfig &config) -> Ptr<FloatSpinnerCommandInput> {

    auto spinner = Ptr<FloatSpinnerCommandInput>{
        children->addFloatSpinnerCommandInput(
            config.id, config.name, config.unit_type, config.minimum,
            config.maximum, config.step, config.initial_value
        )
    };

    auto validator = [spinner] {
        return spinner->value() >= spinner->minimumValue();
    };
    m_validators.emplace_back(validator);

    addInputControl(config.lookup, spinner);

    return spinner;
}

void CreateDialog::createPanelTable(
    const Ptr<CommandInputs> &inputs
) {
    auto table = initializePanelTable(inputs);

    auto top = addPanelTableRow<DialogTopInputs, DialogTopThickness>(inputs, table, m_top_row);
    top.save<DialogTopPanel>();

    auto bottom = addPanelTableRow<DialogBottomInputs, DialogBottomThickness>(inputs, table, m_bottom_row);
    bottom.save<DialogBottomPanel>();

    auto left = addPanelTableRow<DialogLeftInputs, DialogLeftThickness>(inputs, table, m_left_row);
    left.save<DialogLeftPanel>();

    auto right = addPanelTableRow<DialogRightInputs, DialogRightThickness>(inputs, table, m_right_row);
    right.save<DialogRightPanel>();

    auto front = addPanelTableRow<DialogFrontInputs, DialogFrontThickness>(inputs, table, m_front_row);
    front.save<DialogFrontPanel>();

    auto back = addPanelTableRow<DialogBackInputs, DialogBackThickness>(inputs, table, m_back_row);
    back.save<DialogBackPanel>();
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
    for (const auto &title: m_dimensions_table.titles) {
        auto title_input = table->commandInputs()->addTextBoxCommandInput(
            title.id, title.name, "<b>" + title.label + "</b>", 1, true
        );
        table->addCommandInput(title_input, 0, title.column, 0, title.span);
    }
}

template<class T, class U>
auto CreateDialog::addPanelTableRow(
    const Ptr<CommandInputs> &inputs,
    Ptr<adsk::core::TableCommandInput> &table,
    const DimensionTableRow &row
) -> CreatePanelOverrideRow {
    auto is_metric         = m_configuration.ctx<DialogModelingUnits>().value;
    auto default_thickness = m_configuration.ctx<DialogThicknessInput>().control;
    auto row_num           = table->rowCount();

    auto label_control    = addPanelLabelControl(inputs, row);
    auto enable_control   = addPanelEnableControl(inputs, row);
    auto override_control = addPanelOverrideControl(inputs, row);
    override_control->isEnabled(enable_control->value());

    const auto &defaults = row.thickness.defaults.at(is_metric);

    auto thickness_control = inputs->addFloatSpinnerCommandInput(
        row.thickness.id, "", defaults.unit_type, defaults.minimum,
        defaults.maximum, defaults.step, defaults.initial_value
    );
    thickness_control->isEnabled(enable_control->value() && override_control->value());

    table->addCommandInput(label_control, row_num, 0, 0, 0);
    table->addCommandInput(enable_control, row_num, 2, 0, 0);
    table->addCommandInput(override_control, row_num, 5, 0, 0);
    table->addCommandInput(thickness_control, row_num, 7, 0, 0);

    auto override_row = CreatePanelOverrideRow(m_configuration, row.label.name, row.priority, row.orientation);
    auto thickness_swap = override_row.createRow<T, U>(default_thickness, label_control, enable_control, override_control, thickness_control);

    auto thickness_toggle = [override_control, thickness_control] {
        thickness_control->isEnabled(override_control->value() && override_control->isEnabled());
    };

    auto override_toggle = [override_control, enable_control] {
        override_control->isEnabled(enable_control->value());
    };

    auto follow_thickness = [this, override_control, thickness_control] {
        auto const &default_thickness = m_configuration.ctx<DialogThicknessInput>().control;

        auto thickness_enabled = override_control->value();

        if (thickness_enabled) {
            return;
        }

        thickness_control->value(default_thickness->value());
    };

    addInputControl(row.override.lookup, override_control, {thickness_swap, thickness_toggle, follow_thickness});
    addInputControl(row.enable.lookup, enable_control, {thickness_toggle, override_toggle});
    addInputControl(row.thickness.lookup, thickness_control);
    addInputHandler(DialogInputs::Thickness, follow_thickness);
    addCollisionHandler(row.thickness.lookup);

    return override_row;
}

auto CreateDialog::addPanelLabelControl(const Ptr<CommandInputs> &inputs, const DimensionTableRow &row) -> Ptr<TextBoxCommandInput> {
    auto label_control = inputs->addTextBoxCommandInput(
        row.label.id, row.label.name, "<b>" + row.label.name + "</b>", 1, true
    );
    return label_control;
}

auto CreateDialog::addPanelEnableControl(const Ptr<CommandInputs> &inputs, const DimensionTableRow &row) -> Ptr<BoolValueCommandInput> {
    auto enable_control = inputs->addBoolValueInput(
        row.enable.id, row.enable.name, true, "", row.enable.default_value
    );
    return enable_control;
}

auto CreateDialog::addPanelOverrideControl(const Ptr<CommandInputs> &inputs, const DimensionTableRow &row) -> Ptr<BoolValueCommandInput> {
    auto override_control = inputs->addBoolValueInput(
        row.override.id, row.override.name, true, "", false
    );
    return override_control;
}

void CreateDialog::createDividerInputs(const Ptr<CommandInputs> &inputs) {
    auto divider_joint = inputs->addDropDownCommandInput("dividerLapCommandInput", "Top Joint", TextListDropDownStyle);
    m_configuration.set<DialogDividerJointInput>(divider_joint);

    auto const &joint_items = divider_joint->listItems();
    joint_items->add("Length Dividers", false);
    joint_items->add("Width Dividers", true);
    divider_joint->maxVisibleItems(2);

    addInputControl(
        DialogInputs::DividerLapInput, divider_joint, [this]() {
            update(m_configuration.ctx<DialogLengthDividerCountInput>().control);
//            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
        }
    );

    auto length_divider_outside_joint = inputs->addDropDownCommandInput("lengthDividerOutsideJointInput", "Length Divider Joint", TextListDropDownStyle);
    m_configuration.set<DialogLengthDividerJointInput>(length_divider_outside_joint);

    auto const &length_items = length_divider_outside_joint->listItems();
    length_items->add("Tenon", true);
    length_items->add("Half Lap", false);
    length_items->add("Box Joint", false);
    length_divider_outside_joint->maxVisibleItems(3);

    addInputControl(
        DialogInputs::LengthDividerJointInput, length_divider_outside_joint, [this]() {
            update(m_configuration.ctx<DialogLengthDividerCountInput>().control);
        }
    );

//    m_panel_registry.view<JointLengthOrientation, InsideJointPattern, OutsidePanel>().each(
//        [this](
//            auto entity, auto const &orientation, auto const &pattern_position, auto const &panel_position
//        ) {
//            auto const &length_divider_outside_joint = m_configuration.ctx<DialogLengthDividerJointInput>().control;
//            m_panel_registry.emplace<LengthJointInput>(entity, length_divider_outside_joint);
//        }
//    );

    auto length = adsk::core::Ptr<IntegerSpinnerCommandInput>{
        inputs->addIntegerSpinnerCommandInput(
            "lengthDividerCommandInput", "(#) Length Dividers", 0, 25, 1, 0
        )
    };
    m_configuration.set<DialogLengthDividerCountInput>(length);

    addInputControl(
        DialogInputs::LengthDividerCount, length, [this]() {
            auto const &divider_joint                = m_configuration.ctx<DialogDividerJointInput>().control;
            auto const &length_divider_outside_joint = m_configuration.ctx<DialogLengthDividerJointInput>().control;
            auto const divider_count                 = m_configuration.ctx<DialogLengthDividerCountInput>().control->value();
            auto const divider_length                = m_configuration.ctx<DialogLengthInput>().control->value();

            auto old_view = m_configuration.view<LengthDivider>();
            m_configuration.destroy(old_view.begin(), old_view.end());

            auto static_view = m_configuration.view<LengthDividerJoint>();
            m_configuration.destroy(static_view.begin(), static_view.end());

//            auto const outside_joint_type = length_divider_outside_joint->selectedItem()->index();
//            auto const inside_joint_type  = divider_joint->selectedItem()->index();
//
//            auto joint_selector = std::map<int, JointPatternType>{
//                {0, JointPatternType::LapJoint},
//                {1, JointPatternType::LapJoint}
//            };
//            auto joints         = addInsideJoints(AxisFlag::Length, joint_selector[inside_joint_type], outside_joint_type);

            auto dividers = Dividers<LengthDivider, DialogLengthDividerCountInput>(m_configuration, m_app);
            dividers.create(AxisFlag::Length, "Length", divider_length, 3);
            dividers.addOrientation<LengthOrientation>();
            dividers.addMaxOffset<DialogLengthInput>();

            m_systems->updateCollisions();
            m_systems->findJoints<Panel, LengthDivider, LengthDividerJoint>();
            m_systems->updateJointPatternInputs<LengthDividerJoint, DialogLengthDividerJointInput>();
            m_systems->updateJointDirection<LengthDividerJoint>(Position::Outside, Position::Inside, JointDirectionType::Normal);
            m_systems->updateJointDirection<LengthDividerJoint>(Position::Inside, Position::Outside, JointDirectionType::Inverted);
            m_systems->updateJointDirectionInputs<LengthDividerJoint, DialogDividerJointInput>();
            m_systems->postUpdate();
        }
    );
//    addCollisionHandler(DialogInputs::LengthDividerCount);

//    auto width_divider_outside_joint = inputs->addDropDownCommandInput("widthDividerOutsideJointInput", "Width Divider Joint", TextListDropDownStyle);
//    m_configuration.set<DialogWidthDividerJointInput>(width_divider_outside_joint);
//
//    auto const &width_items = width_divider_outside_joint->listItems();
//    width_items->add("Tenon", true);
//    width_items->add("Half Lap", false);
//    width_items->add("Box Joint", false);
//    width_divider_outside_joint->maxVisibleItems(3);
//
//    addInputControl(
//        DialogInputs::WidthDividerJointInput, width_divider_outside_joint, [this]() {
//            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
//        }
//    );
//
//    m_panel_registry.view<JointWidthOrientation, InsideJointPattern, OutsidePanel>().each(
//        [this](
//            auto entity, auto const &orientation, auto const &pattern_position, auto const &panel_position
//        ) {
//            auto const &width_divider_outside_joint = m_configuration.ctx<DialogWidthDividerJointInput>().control;
//            m_panel_registry.emplace<WidthJointInput>(entity, width_divider_outside_joint);
//        }
//    );
//
//    auto width = adsk::core::Ptr<IntegerSpinnerCommandInput>{
//        inputs->addIntegerSpinnerCommandInput(
//            "widthDividerCommandInput", "(#) Width Dividers", 0, 25, 1, 0
//        )
//    };
//    m_configuration.set<DialogWidthDividerCountInput>(width);
//
//    addInputControl(
//        DialogInputs::WidthDividerCount, width, [this]() {
//            auto const &divider_joint               = m_configuration.ctx<DialogDividerJointInput>().control;
//            auto const &width_divider_outside_joint = m_configuration.ctx<DialogWidthDividerJointInput>().control;
//            auto const divider_count                = m_configuration.ctx<DialogWidthDividerCountInput>().control->value();
//            auto const divider_length               = m_configuration.ctx<DialogWidthInput>().control->value();
//
//            auto const outside_joint_type = width_divider_outside_joint->selectedItem()->index();
//            auto const inside_joint_type  = divider_joint->selectedItem()->index();
//
//            auto old_view = m_panel_registry.view<InsidePanel, WidthOrientation>();
//            m_panel_registry.destroy(old_view.begin(), old_view.end());
//
//            auto joint_selector = std::map<int, JointPatternType>{
//                {0, JointPatternType::LapJoint},
//                {1, JointPatternType::LapJoint}
//            };
//            auto joints         = addInsideJoints(AxisFlag::Width, joint_selector[inside_joint_type], outside_joint_type);

//            auto dividers = Dividers<WidthOrientation>(m_panel_registry, m_configuration, m_app, joints);
//            dividers.create("Width", divider_count, divider_length, AxisFlag::Width);
//        }
//    );

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
//
//    addInputHandler(
//        DialogInputs::Length, [this]() {
//            update(m_configuration.ctx<DialogLengthDividerCountInput>().control);
//            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
//        }
//    );
//
//    addInputHandler(
//        DialogInputs::Width, [this]() {
//            update(m_configuration.ctx<DialogLengthDividerCountInput>().control);
//            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
//        }
//    );
//
//    addInputHandler(
//        DialogInputs::Height, [this]() {
//            update(m_configuration.ctx<DialogLengthDividerCountInput>().control);
//            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
//        }
//    );

}

//axisJointTypePositionMap CreateDialog::addInsideJoints(
//    const AxisFlag panel_orientation,
//    const JointPatternType inside_joint_type,
//    const int outside_joint_type
//) {
//    using jointTypeMap = std::map<entities::JointPatternType, std::vector<entities::Position>>;
//    using selectorJointTypeMap = std::map<int, jointTypeMap>;
//    using axisSelectorMap = std::map<AxisFlag, selectorJointTypeMap>;
//
//    auto finger_orientation = axisJointTypePositionMap{
//    };
//
//    auto inverse_toplap_selector    = selectorJointTypeMap{
//        {
//            2, {
//                   {JointPatternType::Inverse, {Position::Outside}},
//                   {JointPatternType::Corner, {Position::Outside}}
//               }},
//        {
//            1, {
//                   {JointPatternType::TopLap,  {Position::Outside}},
//               }},
//        {
//            0, {
//                   {JointPatternType::Tenon,   {Position::Outside}},
//               }}
//    };
//    auto inverse_trim_selector      = selectorJointTypeMap{
//        {
//            2, {
//                   {JointPatternType::Inverse, {Position::Outside}},
//                   {JointPatternType::Corner, {Position::Outside}}
//               }},
//        {
//            1, {
//                   {JointPatternType::Trim,    {Position::Outside}},
//               }},
//        {
//            0, {
//                   {JointPatternType::Trim,    {Position::Outside}},
//               }}
//    };
//    auto hgt_outside_joint_selector = axisSelectorMap{
//        {AxisFlag::Length, inverse_trim_selector},
//        {AxisFlag::Width,  inverse_trim_selector},
//        {AxisFlag::Height, inverse_toplap_selector}
//    };
//    auto lw_outside_joint_selector  = axisSelectorMap{
//        {AxisFlag::Length, inverse_toplap_selector},
//        {AxisFlag::Width,  inverse_toplap_selector},
//        {AxisFlag::Height, inverse_trim_selector}
//    };
//
//    if (panel_orientation == AxisFlag::Length) {
//        finger_orientation[AxisFlag::Height] = hgt_outside_joint_selector[panel_orientation][outside_joint_type];
//        finger_orientation[AxisFlag::Width]  = lw_outside_joint_selector[panel_orientation][outside_joint_type];
//        finger_orientation[AxisFlag::Width][inside_joint_type].emplace_back(Position::Inside);
//    } else if (panel_orientation == AxisFlag::Width) {
//        finger_orientation[AxisFlag::Height] = hgt_outside_joint_selector[panel_orientation][outside_joint_type];
//        finger_orientation[AxisFlag::Length] = lw_outside_joint_selector[panel_orientation][outside_joint_type];
//        finger_orientation[AxisFlag::Length][inside_joint_type].emplace_back(Position::Inside);
//    } else if (panel_orientation == AxisFlag::Height) {
//        finger_orientation[AxisFlag::Length][inside_joint_type].emplace_back(Position::Inside);
//        finger_orientation[AxisFlag::Length] = hgt_outside_joint_selector[panel_orientation][outside_joint_type];
//        finger_orientation[AxisFlag::Width]  = hgt_outside_joint_selector[panel_orientation][outside_joint_type];
//    }
//
//    return finger_orientation;
//}

void CreateDialog::createOffsetInputs(const Ptr<CommandInputs> &inputs) {
    auto const inset_panels = inputs->addDropDownCommandInput("insetPanelsCommandInput", "Inset Panels", TextListDropDownStyle);
    m_configuration.set<DialogInsetPanelsInput>(inset_panels);

    auto const &joint_items = inset_panels->listItems();
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
    m_configuration.set<PanelOffsetInput>(bottom_offset);
}

auto CreateDialog::createStandardJointTable(const adsk::core::Ptr<CommandInputs> &inputs) -> adsk::core::Ptr<TableCommandInput> {
    auto table = inputs->addTableCommandInput(
        "jointTableCommandInput", "Joints", 0, "1:1:1:1"
    );
    table->maximumVisibleRows(17);
    table->minimumVisibleRows(17);
    table->tablePresentationStyle(itemBorderTablePresentationStyle);

    auto const first_label    = table->commandInputs()->addTextBoxCommandInput("firstJointLabelInput", "First", "<b>First</b>", 1, true);
    auto const second_label   = table->commandInputs()->addTextBoxCommandInput("secondJointLabelInput", "Second", "<b>Second</b>", 1, true);
    auto const pattern_label  = table->commandInputs()->addTextBoxCommandInput("jointPatternLabelInput", "Pattern", "<b>Pattern</b>", 1, true);
    auto const type_label     = table->commandInputs()->addTextBoxCommandInput("jointPatternLabelInput", "Joint Type", "<b>Joint Type</b>", 1, true);
//    auto const pos_label      = table->commandInputs()->addTextBoxCommandInput("posLabelInput", "Panel Offset", "<b>Panel Offset</b>", 1, true);
//    auto const jos_label      = table->commandInputs()->addTextBoxCommandInput("josLabelInput", "Joint Offset", "<b>Joint Offset</b>", 1, true);
//    auto const distance_label = table->commandInputs()->addTextBoxCommandInput("distanceLabelInput", "Distance", "<b>Distance</b>", 1, true);

    table->addCommandInput(first_label, 0, 0);
    table->addCommandInput(second_label, 0, 1);
    table->addCommandInput(pattern_label, 0, 2);
    table->addCommandInput(type_label, 0, 3);
//    table->addCommandInput(pos_label, 0, 4);
//    table->addCommandInput(jos_label, 0, 5);
//    table->addCommandInput(distance_label, 0, 6);
    return table;
}

void CreateDialog::createPreviewTable(const adsk::core::Ptr<CommandInputs> &inputs) {
    auto table = inputs->addTableCommandInput(
        "previewTableCommandInput", "Preview", 0, "1:5:1:5"
    );

    table->maximumVisibleRows((int) 1);
    table->minimumVisibleRows((int) 1);
    table->isEnabled(false);
    table->tablePresentationStyle(transparentBackgroundTablePresentationStyle);

    auto const fast_preview = table->commandInputs()->addBoolValueInput("fastPreviewCommandInput", "Fast Preview", true, "", true);
    auto const fast_label   = table->commandInputs()->addTextBoxCommandInput("fastPreviewLabelInput", "Fast Preview", "Fast Preview", 1, true);
    auto const full_preview = table->commandInputs()->addBoolValueInput("fullPreviewCommandInput", "Full Preview", true, "", false);
    auto const full_label   = table->commandInputs()->addTextBoxCommandInput("fastPreviewLabelInput", "Full Preview", "Full Preview", 1, true);

    m_configuration.set<DialogFastPreviewMode>(fast_preview);
    m_configuration.set<DialogFastPreviewLabel>(fast_label);
    m_configuration.set<DialogFullPreviewMode>(full_preview);
    m_configuration.set<DialogFullPreviewLabel>(full_label);

    table->addCommandInput(fast_preview, 0, 0);
    table->addCommandInput(fast_label, 0, 1);
    table->addCommandInput(full_preview, 0, 2);
    table->addCommandInput(full_label, 0, 3);

    addInputControl(DialogInputs::FastPreviewLabel, fast_label);
    addInputControl(
        DialogInputs::FastPreview, fast_preview, [this]() {
            auto const &fast_preview = m_configuration.ctx<DialogFastPreviewMode>().control;
            auto const &full_preview = m_configuration.ctx<DialogFullPreviewMode>().control;

            if (fast_preview->value()) {
                full_preview->value(false);
            }
        }
    );

    addInputControl(DialogInputs::FullPreviewLabel, full_label);
    addInputControl(
        DialogInputs::FullPreview, full_preview, [this]() {
            auto const &fast_preview = m_configuration.ctx<DialogFastPreviewMode>().control;
            auto const &full_preview = m_configuration.ctx<DialogFullPreviewMode>().control;

            if (full_preview->value()) {
                fast_preview->value(false);
            }
        }
    );
}

template<class T, class U>
bool validateDimension(entt::registry &m_configuration, AxisFlag axis_flag) {
    auto msg_map = std::unordered_map<AxisFlag, std::string>{
        {AxisFlag::Height, "<span style=\" font-weight:600; color:#ff0000;\">Height</span> should be greater than the thickness of top to bottom panels."},
        {AxisFlag::Length, "<span style=\" font-weight:600; color:#ff0000;\">Length</span> should be greater than the thickness of left to right panels."},
        {AxisFlag::Width,  "<span style=\" font-weight:600; color:#ff0000;\">Width</span> should be greater than the thickness of front to back panels."},
    };

    auto dimension_control = m_configuration.ctx<T>().control;

    auto minimum = 0.0;

    auto view = m_configuration.view<const U, const DialogPanelEnable, const DialogPanelThickness>().proxy();
    for (auto &&[entity, dimension, enable, thickness]: view) {
        minimum += thickness.control->value() * enable.control->value();
    }

    auto value = dimension_control->value();
    if (value > minimum) { return true; }

    m_configuration.set<DialogErrorMessage>(msg_map[axis_flag]);

    return false;
}

void CreateDialog::addMinimumAxisDimensionChecks() {
    auto validator = [this]() {
        auto length_ok = validateDimension<DialogLengthInput, LengthOrientation>(m_configuration, AxisFlag::Length);
        auto width_ok  = validateDimension<DialogWidthInput, WidthOrientation>(m_configuration, AxisFlag::Width);
        auto height_ok = validateDimension<DialogHeightInput, HeightOrientation>(m_configuration, AxisFlag::Height);

        return length_ok && width_ok && height_ok;
    };
    m_validators.emplace_back(validator);
}

void CreateDialog::addMaximumKerfCheck() {
    auto validator = [this]() {
        auto kerf = m_configuration.ctx<DialogKerfInput>().control->value();

        bool kerf_ok = true;

        auto view = m_configuration.view<const DialogPanelThickness>().proxy();
        for (auto &&[entity, thickness]: view) {
            kerf_ok = kerf_ok && (thickness.control->value() > kerf);
        }

        if (kerf_ok) { return true; }

        m_configuration.set<DialogErrorMessage>(
            "<span style=\" font-weight:600; color:#ff0000;\">Kerf</span> is larger than the smallest panel thickness."
        );

        return false;
    };
    m_validators.emplace_back(validator);
}

void CreateDialog::addMinimumFingerWidthCheck() {
    auto validator = [this]() {
        auto finger_width = m_configuration.ctx<DialogFingerWidthInput>().control->value();

        bool finger_ok = true;

        auto view = m_configuration.view<const DialogPanelThickness>().proxy();
        for (auto &&[entity, thickness]: view) {
            finger_ok = finger_ok && (thickness.control->value() < finger_width);
        }

        if (finger_ok) { return true; }

        m_configuration.set<DialogErrorMessage>(
            "<span style=\" font-weight:600; color:#ff0000;\">Finger width</span> should be larger than the largest panel thickness."
        );

        return false;
    };
    m_validators.emplace_back(validator);
}

void CreateDialog::addMinimumPanelCountCheck() {
    auto validator = [this]() {
        int thickness_count = 0;

        auto view = m_configuration.view<const DialogPanelEnable>().proxy();
        for (auto &&[entity, enable]: view) {
            thickness_count += enable.control->value();
        }

        if (thickness_count < 2) {
            m_configuration.set<DialogErrorMessage>(
                "<span style=\" font-weight:600; color:#ff0000;\">A minimum of two panels must be selected."
            );
            return false;
        }

        return true;
    };
    m_validators.emplace_back(validator);
}

void CreateDialog::populateJointTable(Ptr<TableCommandInput> &table) {

    auto inputs  = table->commandInputs();
    auto row_num = 1;

    auto view = m_configuration.view<entities::DialogJoints, entities::DialogJointPattern, entities::DialogPanelCollisionData>().proxy();
    for (auto &&[entity, joints, pattern, data]: view) {
        // Account for -0 in strings.
        auto pos_val  = data.first.panel_offset == 0 ? 0 : data.first.panel_offset;
        auto jos_val  = data.first.joint_offset == 0 ? 0 : data.first.joint_offset;
        auto dist_val = data.first.distance == 0 ? 0 : data.first.distance;

        auto row_str  = std::to_string(row_num);
        auto pos_str  = std::to_string(pos_val);
        auto jos_str  = std::to_string(jos_val);
        auto dist_str = std::to_string(dist_val);

        auto first  = inputs->addTextBoxCommandInput("jointRowFirst" + row_str, joints.first.panel.name, joints.first.panel.name, 1, true);
        auto second = inputs->addTextBoxCommandInput("jointRowSecond" + row_str, joints.second.panel.name, joints.second.panel.name, 1, true);

        auto dropdown = inputs->addDropDownCommandInput("jointRowPattern" + row_str, "Joint Direction", TextListDropDownStyle);
        auto const &pattern_items = dropdown->listItems();
        pattern_items->add("Normal", true);
        pattern_items->add("Inverse", false);
        dropdown->maxVisibleItems(2);
        auto cmd_input = Ptr<CommandInput>{dropdown};

        m_configuration.emplace<DialogJointDirectionInputs>(entity, DialogJointDirectionInput{dropdown}, DialogJointDirectionInput{dropdown, true});

        auto type_dropdown = inputs->addDropDownCommandInput("jointRowType" + row_str, "Joint Pattern", TextListDropDownStyle);
        auto const &type_items = type_dropdown->listItems();
        type_items->add("Box Joint", true);
        type_items->add("Lap Joint", false);
        type_items->add("Tenon", false);
        type_items->add("Double Tenon", false);
        type_items->add("Triple Tenon", false);
        type_items->add("Quad Tenon", false);
        type_items->add("Trim", false);
//            type_items->add("Pattern", false);
        type_items->add("None", false);
        type_dropdown->maxVisibleItems(7);

        m_configuration.emplace<DialogJointPatternInput>(entity, type_dropdown);

        table->addCommandInput(first, row_num, 0);
        table->addCommandInput(second, row_num, 1);
        table->addCommandInput(dropdown, row_num, 2);
        table->addCommandInput(type_dropdown, row_num, 3);

        row_num += 1;
    }

    addCollisionHandler(DialogInputs::Length);
    addCollisionHandler(DialogInputs::Width);
    addCollisionHandler(DialogInputs::Height);
    addCollisionHandler(DialogInputs::Thickness);

}

void CreateDialog::addCollisionHandler(DialogInputs reference) {
    auto handler = [this]() {
        m_systems->updateCollisions();
        m_systems->postUpdate();
    };

    addInputHandler(reference, handler);
}

bool CreateDialog::validate(const adsk::core::Ptr<CommandInputs> &inputs) {
    auto m_error_message = m_configuration.ctx<DialogErrorMessage>().value;
    m_error->formattedText(m_error_message);

    auto results = all_of(m_validators.begin(), m_validators.end(), [](const std::function<bool()> &v) { return v(); });

    m_error->isVisible(!results);

    return results;
}

bool CreateDialog::update(const adsk::core::Ptr<CommandInput> &cmd_input) {
    auto exists = std::find(m_ignore_updates.begin(), m_ignore_updates.end(), cmd_input->id()) != m_ignore_updates.end();
    if (exists) { return false; }

    auto handlers = m_handlers[cmd_input->id()];

    for (auto &handler: handlers) {
        handler();
    }

    m_systems->postUpdate();

    return true;
}

void CreateDialog::initializePanels() {

    m_panel_registry.clear();

    std::map<entt::entity, std::set<entt::entity>> first_index = {};
    std::map<entt::entity, std::set<entt::entity>> second_index = {};

    auto kerf = m_configuration.ctx<DialogKerfInput>().control;

    auto master_view = m_configuration.view<Enabled, FingerPattern, FingerWidth, DialogJoints, DialogPanelCollisionData, DialogPanels, JointPattern, PanelPositions, JointDirections>().proxy();
    for (auto &&[entity, enabled, fm, fw, joints, collision_data, panels, pt, pp, jd]: master_view) {
        auto create_panel = [&, this, finger_mode = fm, finger_width = fw, pattern_type = pt](
            const DialogPanelJoint& joint, const DialogPanelJointData collision, const entt::entity& second_panel, const PanelPositions& positions, const JointDirectionType& joint_direction
        ) {
            auto panel_offset = collision.panel_offset;
            auto joint_distance = collision.distance;
            auto panel_position = positions.first;
            auto joint_position = positions.second;

            auto panel = m_panel_registry.create();
            m_panel_registry.emplace<KerfInput>(panel, kerf);
            m_panel_registry.emplace<FingerPattern>(panel, finger_mode);
            m_panel_registry.emplace<FingerWidth>(panel, finger_width);
            m_panel_registry.emplace<JointPattern>(panel, pattern_type);
            m_panel_registry.emplace<JointPanelOffset>(panel, panel_offset);
            m_panel_registry.emplace<JointPatternDistance>(panel, joint_distance);

            m_panel_registry.emplace<PanelPosition>(panel, panel_position);
            m_panel_registry.emplace<JointPosition>(panel, joint_position);
            m_panel_registry.emplace<JointProfile>(
                panel, panel_position, joint_position, joint_direction, JointPatternType::BoxJoint, FingerPatternType::Automatic, 0, 0.0, 0.0, 0.0, 0.0, AxisFlag::Length, AxisFlag::Length
            );
            m_panel_registry.emplace<JointPatternPosition>(
                panel, panel_position, AxisFlag::Length, JointPatternType::BoxJoint, AxisFlag::Length, joint_position
            );
            m_panel_registry.emplace<JointDirection>(panel, joint_direction);

            m_panel_registry.emplace<PanelOffset>(panel);
            m_panel_registry.emplace<EndReferencePoint>(panel);
            m_panel_registry.emplace<ExtrusionDistance>(panel);
            m_panel_registry.emplace<PanelProfile>(panel);
            m_panel_registry.emplace<StartReferencePoint>(panel);

            PLOG_DEBUG << "Adding panel registry entity for " << joint.panel.name;
            first_index[joint.entity].insert(panel);
            PLOG_DEBUG << joint.panel.name << " now has " << first_index[joint.entity].size() << " elements.";
            second_index[second_panel].insert(panel);
        };

        create_panel(joints.first, collision_data.first, joints.second.entity, pp, jd.first);
        create_panel(joints.second, collision_data.second, joints.first.entity, {pp.second, pp.first}, jd.second);
    }

    auto process_view = m_configuration.view<
        const DialogPanelEnableValue, const DialogPanel, const PanelDimensions, const DialogPanelThickness, const MaxOffsetInput
    >().proxy();
    for (auto &&[entity, enable, panel_data, dimensions, thickness, max_offset]: process_view) {
        PLOG_DEBUG << "Generating panel configuration";
        auto first_panels = first_index[entity];
        auto second_panels = second_index[entity];

        auto parent_panel = m_panel_registry.create();
        m_panel_registry.emplace<Panel>(parent_panel, panel_data.name, panel_data.priority, panel_data.orientation);
        m_panel_registry.emplace<ChildPanels>(parent_panel, first_panels);

        for (auto const &panel: first_panels) {
            PLOG_DEBUG << "Adding enable, panel and dimension data to panel " << panel_data.name;
            PLOG_DEBUG << "Setting thickness to " << std::to_string(thickness.control->value());
            m_panel_registry.emplace<Dimensions>(panel, dimensions.length, dimensions.width, dimensions.height, thickness.control->value());
            m_panel_registry.emplace<MaxOffset>(panel, max_offset.control->value());
            m_panel_registry.emplace<Enabled>(panel, enable.value);
            m_panel_registry.emplace<Panel>(panel, panel_data.name, panel_data.priority, panel_data.orientation);
            m_panel_registry.emplace<Thickness>(panel, thickness.control->value());
            m_panel_registry.emplace<ParentPanel>(panel, parent_panel);
        }

        for (auto const &panel: second_panels) {
            PLOG_DEBUG << "Adding joint name for " << panel_data.name;
            m_panel_registry.emplace<JointEnabled>(panel, enable.value);
            m_panel_registry.emplace<JointName>(panel, panel_data.name);
            m_panel_registry.emplace<JointOrientation>(panel, panel_data.orientation);
            m_panel_registry.emplace<JointThickness>(panel, thickness.control->value());
        }
    }

    auto joint_profile_direction_view = m_panel_registry.view<JointProfile, const JointDirection>().proxy();
    for (auto &&[entity, profile, direction]: joint_profile_direction_view) {
        PLOG_DEBUG << "Setting Joint Profile direction for " << (int)entity << " to " << (int)direction.value;
        profile.joint_direction = direction.value;
    }

    auto joint_profile_position_view = m_panel_registry.view<JointProfile, const PanelPosition, const JointPosition>().proxy();
    for (auto &&[entity, profile, panel, joint]: joint_profile_position_view) {
        PLOG_DEBUG << "Updating joint profile with panel and joint position";
        profile.panel_position = panel.value;
        profile.joint_position = joint.value;
    }

    auto profile_orientation_view = m_panel_registry.view<JointProfile, const Panel, const JointOrientation>().proxy();
    for (auto &&[entity, profile, panel, joint]: profile_orientation_view) {
        PLOG_DEBUG << "Add orientation group for " << panel.name;
        profile.panel_orientation = panel.orientation;
        profile.joint_orientation = joint.axis;
        m_panel_registry.emplace<OrientationGroup>(entity, panel.orientation, joint.axis);
    }

    auto profile_pattern_view = m_panel_registry.view<JointProfile, const JointPattern>().proxy();
    for (auto &&[entity, profile, pattern]: profile_pattern_view) {
        PLOG_DEBUG << "Updating JointProfile pattern";
        profile.joint_type = pattern.value;
    }

    auto pattern_position_view = m_panel_registry.view<JointPatternPosition, const PanelPosition, const JointPosition>().proxy();
    for (auto &&[entity, pattern, panel, joint]: pattern_position_view) {
        PLOG_DEBUG << "Updating joint pattern position with panel and joint position";
        pattern.panel_position = panel.value;
        pattern.joint_position = joint.value;
    }

    auto panel_length_view = m_panel_registry.view<const Panel>().proxy();
    for (auto &&[entity, panel]: panel_length_view) {
        if (panel.orientation != AxisFlag::Length) continue;
        PLOG_DEBUG << "Add length orientation for " << panel.name;
        m_panel_registry.emplace<LengthOrientation>(entity);
    }

    auto panel_width_view = m_panel_registry.view<const Panel>().proxy();
    for (auto &&[entity, panel]: panel_width_view) {
        if (panel.orientation != AxisFlag::Width) continue;
        PLOG_DEBUG << "Add width orientation for " << panel.name;
        m_panel_registry.emplace<WidthOrientation>(entity);
    }

    auto panel_height_view = m_panel_registry.view<const Panel>().proxy();
    for (auto &&[entity, panel]: panel_height_view) {
        if (panel.orientation != AxisFlag::Height) continue;
        PLOG_DEBUG << "Add height orientation for " << panel.name;
        m_panel_registry.emplace<HeightOrientation>(entity);
    }

    auto pattern_automatic_view = m_panel_registry.view<const FingerPattern>().proxy();
    for (auto &&[entity, pattern]: pattern_automatic_view) {
        if (pattern.value != FingerPatternType::Automatic) continue;
        PLOG_DEBUG << "Add automatic finger pattern type.";
        m_panel_registry.emplace<AutomaticFingerPatternType>(entity);
    }

    auto pattern_constant_view = m_panel_registry.view<const FingerPattern>().proxy();
    for (auto &&[entity, pattern]: pattern_constant_view) {
        if (pattern.value != FingerPatternType::Constant) continue;
        PLOG_DEBUG << "Add constant finger pattern type.";
        m_panel_registry.emplace<ConstantFingerPatternType>(entity);
    }

    auto pattern_none_view = m_panel_registry.view<const FingerPattern>().proxy();
    for (auto &&[entity, pattern]: pattern_none_view) {
        if (pattern.value != FingerPatternType::None) continue;
        PLOG_DEBUG << "Add no finger pattern type.";
        m_panel_registry.emplace<NoFingerPatternType>(entity);
    }

    auto direction_normal_view = m_panel_registry.view<const JointDirection>().proxy();
    for (auto &&[entity, direction]: direction_normal_view) {
        if (direction.value != JointDirectionType::Normal) continue;
        PLOG_DEBUG << "Add normal joint direction.";
        m_panel_registry.emplace<NormalJointDirection>(entity);
    }

    auto direction_inverse_view = m_panel_registry.view<const JointDirection>().proxy();
    for (auto &&[entity, direction]: direction_inverse_view) {
        if (direction.value != JointDirectionType::Inverted) continue;
        PLOG_DEBUG << "Add inverse joint direction.";
        m_panel_registry.emplace<InverseJointDirection>(entity);
    }

    auto pattern_boxjoint_view = m_panel_registry.view<const JointPattern>().proxy();
    for (auto &&[entity, pattern]: pattern_boxjoint_view) {
        if (pattern.value != JointPatternType::BoxJoint) continue;
        PLOG_DEBUG << "Updating BoxJointPattern";
        m_panel_registry.emplace<BoxJointPattern>(entity);
    }

    auto pattern_lapjoint_view = m_panel_registry.view<const JointPattern>().proxy();
    for (auto &&[entity, pattern]: pattern_lapjoint_view) {
        if (pattern.value != JointPatternType::LapJoint) continue;
        PLOG_DEBUG << "Updating LapJointPattern";
        m_panel_registry.emplace<LapJointPattern>(entity);
    }

    auto pattern_tenon_view = m_panel_registry.view<const JointPattern>().proxy();
    for (auto &&[entity, pattern]: pattern_tenon_view) {
        if (pattern.value != JointPatternType::Tenon) continue;
        PLOG_DEBUG << "Updating TenonJointPattern";
        m_panel_registry.emplace<TenonJointPattern>(entity);
    }

    auto pattern_doubletenon_view = m_panel_registry.view<const JointPattern>().proxy();
    for (auto &&[entity, pattern]: pattern_doubletenon_view) {
        if (pattern.value != JointPatternType::DoubleTenon) continue;
        PLOG_DEBUG << "Updating DoubleTenontJointPattern";
        m_panel_registry.emplace<DoubleTenonJointPattern>(entity);
    }

    auto pattern_tripletenon_view = m_panel_registry.view<const JointPattern>().proxy();
    for (auto &&[entity, pattern]: pattern_tripletenon_view) {
        if (pattern.value != JointPatternType::TripleTenon) continue;
        PLOG_DEBUG << "Updating TripleTenonJointPattern";
        m_panel_registry.emplace<TripleTenonJointPattern>(entity);
    }

    auto pattern_quadtenon_view = m_panel_registry.view<const JointPattern>().proxy();
    for (auto &&[entity, pattern]: pattern_quadtenon_view) {
        if (pattern.value != JointPatternType::QuadTenon) continue;
        PLOG_DEBUG << "Updating QuadTenonJointPattern";
        m_panel_registry.emplace<QuadTenonJointPattern>(entity);
    }

    auto pattern_trimjoint_view = m_panel_registry.view<const JointPattern>().proxy();
    for (auto &&[entity, pattern]: pattern_trimjoint_view) {
        if (pattern.value != JointPatternType::Trim) continue;
        PLOG_DEBUG << "Updating TrimJointPattern";
        m_panel_registry.emplace<TrimJointPattern>(entity);
    }

    auto joint_enabled_view = m_panel_registry.view<const JointEnabledInput>().proxy();
    for (auto &&[entity, enabled]: joint_enabled_view) {
        PLOG_DEBUG << "Updating JointEnabled value";
        m_panel_registry.emplace<JointEnabled>(entity, enabled.control->value());
    }

    m_panel_registry.view<
        Panel,
        EnableInput,
        DimensionsInputs,
        ThicknessInput,
        JointPanelOffset,
        JointPatternDistance,
        JointName,
        JointDirection,
        JointOrientation,
        PanelPosition,
        JointPosition,
        JointThickness,
        JointPattern,
        JointEnabled
    >().each([](
        auto entity,
        auto const &panel,
        auto const &enable,
        auto const &dimensions,
        auto const &thickness,
        auto const &joint_panel_offset,
        auto const &distance,
        auto const &joint_name,
        auto const &joint_direction,
        auto const &joint_orientation,
        auto const &panel_position,
        auto const &joint_position,
        auto const &joint_thickness,
        auto const &joint_pattern,
        auto const &joint_enabled
    ){
        PLOG_DEBUG << "<<<<<<<< " << panel.name << " jointed with " << joint_name.value << " <<<<<<<<";
        PLOG_DEBUG << " panel is enabled == " << enable.control->value();
        PLOG_DEBUG << " joint is enabled == " << joint_enabled.value;
        PLOG_DEBUG << " thickness is " << thickness.control->value();
        PLOG_DEBUG << " length is " << dimensions.length->value();
        PLOG_DEBUG << " width is " << dimensions.width->value();
        PLOG_DEBUG << " height is " << dimensions.height->value();
        PLOG_DEBUG << " orientation is " << (int)panel.orientation;
        PLOG_DEBUG << " joint panel offset == " << joint_panel_offset.value;
        PLOG_DEBUG << " joint distance == " << distance.value << " along orientation " << (int)joint_orientation.axis;
        PLOG_DEBUG << " joint direction == " << (int)joint_direction.value;
        PLOG_DEBUG << " position is " << (int) panel_position.value;
        PLOG_DEBUG << " joint position is " << (int) joint_position.value;
        PLOG_DEBUG << " joint thickness is " << joint_thickness.value;
        PLOG_DEBUG << " joint pattern is " << (int) joint_pattern.value;
        PLOG_DEBUG << ">>>>>>>> " << panel.name << " jointed with " << joint_name.value << " >>>>>>>>";
    });
}
