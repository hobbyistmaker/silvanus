//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/22/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//

#include "dialog.hpp"

#include "dividers.hpp"

#include "entities/AxisFlag.hpp"
#include "entities/ChildPanels.hpp"
#include "entities/Dimensions.hpp"
#include "entities/Enabled.hpp"
#include "entities/EnableInput.hpp"
#include "entities/EndReferencePoint.hpp"
#include "entities/FingerMode.hpp"
#include "entities/FingerWidth.hpp"
#include "entities/FingerWidthInput.hpp"
#include "entities/FingerPatternType.hpp"
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
#include "entities/PanelDimensionInputs.hpp"
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
using namespace silvanus::generatebox::dialog;

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

    auto const &insets = inputs->addTabCommandInput("insertPanelsTabInput", "Insets");
    insets->tooltip("Panel Insets");
    createOffsetInputs(insets->children());

    auto const &joints = inputs->addTabCommandInput("panelJointsTabInput", "Joints");
    joints->tooltip("Panel Joints");
    auto joint_table = createJointTable(joints->children());

    createPreviewTable(inputs);
    m_error = inputs->addTextBoxCommandInput("errorMessageCommandInput", "", "", 2, true);
    m_error->isVisible(false);

    m_ignore_updates.emplace_back(m_error->id());

    addMinimumAxisDimensionChecks();
    addMaximumKerfCheck();
    addMinimumFingerWidthCheck();
    addMinimumPanelCountCheck();

    updatePanelOrientations();
    createPanelDimensions();
    projectPlanes();
    findJoints();
    populateJointTable(joint_table);

    postUpdate();
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
    m_configuration.set<DialogTopPanel>(top);

    auto bottom = addPanelTableRow<DialogBottomInputs, DialogBottomThickness>(inputs, table, m_bottom_row);
    m_configuration.set<DialogBottomPanel>(bottom);

    auto left = addPanelTableRow<DialogLeftInputs, DialogLeftThickness>(inputs, table, m_left_row);
    m_configuration.set<DialogLeftPanel>(left);

    auto right = addPanelTableRow<DialogRightInputs, DialogRightThickness>(inputs, table, m_right_row);
    m_configuration.set<DialogRightPanel>(right);

    auto front = addPanelTableRow<DialogFrontInputs, DialogFrontThickness>(inputs, table, m_front_row);
    m_configuration.set<DialogFrontPanel>(front);

    auto back = addPanelTableRow<DialogBackInputs, DialogBackThickness>(inputs, table, m_back_row);
    m_configuration.set<DialogBackPanel>(back);

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
) -> entt::entity {
    auto entity = m_configuration.create();

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

    m_configuration.emplace<DialogPanelEnable>(entity, enable_control);
    m_configuration.emplace<DialogPanelInputs>(entity, label_control, enable_control, override_control, thickness_control);
    m_configuration.emplace<DialogPanelLabel>(entity, label_control);
    m_configuration.emplace<DialogPanelOverride>(entity, override_control);
    m_configuration.emplace<DialogPanelOverrideThickness>(entity, thickness_control);
    m_configuration.emplace<DialogPanelThickness>(entity, default_thickness);
    m_configuration.emplace<OutsidePanel>(entity);
    m_configuration.emplace<PanelPosition>(entity, Position::Outside);
    m_configuration.emplace<DialogPanel>(entity, row.label.name, row.priority, row.orientation);
    m_configuration.emplace<DialogPanelPlanes>(entity);

    m_configuration.set<T>(label_control, enable_control, override_control, thickness_control);
    m_configuration.set<U>(thickness_control);

    auto thickness_swap = [this, entity, override_control, thickness_control] {
        auto const default_thickness = m_configuration.ctx<DialogThicknessInput>().control;

        auto t_control = Ptr<CommandInput>{thickness_control};
        auto d_control = Ptr<CommandInput>{default_thickness};

        auto toggle = override_control->value() ? t_control : d_control;

        m_configuration.replace<DialogPanelThickness>(entity, toggle);
        m_configuration.set<U>(toggle);
    };

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

    return entity;
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
            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
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

    m_panel_registry.view<JointLengthOrientation, InsideJointPattern, OutsidePanel>().each(
        [this](
            auto entity, auto const &orientation, auto const &pattern_position, auto const &panel_position
        ) {
            auto const &length_divider_outside_joint = m_configuration.ctx<DialogLengthDividerJointInput>().control;
            m_panel_registry.emplace<LengthJointInput>(entity, length_divider_outside_joint);
        }
    );

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

            auto old_view = m_panel_registry.view<InsidePanel, LengthOrientation>();
            m_panel_registry.destroy(old_view.begin(), old_view.end());

            auto const outside_joint_type = length_divider_outside_joint->selectedItem()->index();
            auto const inside_joint_type  = divider_joint->selectedItem()->index();

            auto joint_selector = std::map<int, JointPatternType>{
                {0, JointPatternType::LapJoint},
                {1, JointPatternType::LapJoint}
            };
//            auto joints         = addInsideJoints(AxisFlag::Length, joint_selector[inside_joint_type], outside_joint_type);

//            auto dividers = Dividers<LengthOrientation>(m_panel_registry, m_configuration, m_app, joints);
//            dividers.create("Length", divider_count, divider_length, AxisFlag::Length);
        }
    );

    auto width_divider_outside_joint = inputs->addDropDownCommandInput("widthDividerOutsideJointInput", "Width Divider Joint", TextListDropDownStyle);
    m_configuration.set<DialogWidthDividerJointInput>(width_divider_outside_joint);

    auto const &width_items = width_divider_outside_joint->listItems();
    width_items->add("Tenon", true);
    width_items->add("Half Lap", false);
    width_items->add("Box Joint", false);
    width_divider_outside_joint->maxVisibleItems(3);

    addInputControl(
        DialogInputs::WidthDividerJointInput, width_divider_outside_joint, [this]() {
            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
        }
    );

    m_panel_registry.view<JointWidthOrientation, InsideJointPattern, OutsidePanel>().each(
        [this](
            auto entity, auto const &orientation, auto const &pattern_position, auto const &panel_position
        ) {
            auto const &width_divider_outside_joint = m_configuration.ctx<DialogWidthDividerJointInput>().control;
            m_panel_registry.emplace<WidthJointInput>(entity, width_divider_outside_joint);
        }
    );

    auto width = adsk::core::Ptr<IntegerSpinnerCommandInput>{
        inputs->addIntegerSpinnerCommandInput(
            "widthDividerCommandInput", "(#) Width Dividers", 0, 25, 1, 0
        )
    };
    m_configuration.set<DialogWidthDividerCountInput>(width);

    addInputControl(
        DialogInputs::WidthDividerCount, width, [this]() {
            auto const &divider_joint               = m_configuration.ctx<DialogDividerJointInput>().control;
            auto const &width_divider_outside_joint = m_configuration.ctx<DialogWidthDividerJointInput>().control;
            auto const divider_count                = m_configuration.ctx<DialogWidthDividerCountInput>().control->value();
            auto const divider_length               = m_configuration.ctx<DialogWidthInput>().control->value();

            auto const outside_joint_type = width_divider_outside_joint->selectedItem()->index();
            auto const inside_joint_type  = divider_joint->selectedItem()->index();

            auto old_view = m_panel_registry.view<InsidePanel, WidthOrientation>();
            m_panel_registry.destroy(old_view.begin(), old_view.end());

            auto joint_selector = std::map<int, JointPatternType>{
                {0, JointPatternType::LapJoint},
                {1, JointPatternType::LapJoint}
            };
//            auto joints         = addInsideJoints(AxisFlag::Width, joint_selector[inside_joint_type], outside_joint_type);

//            auto dividers = Dividers<WidthOrientation>(m_panel_registry, m_configuration, m_app, joints);
//            dividers.create("Width", divider_count, divider_length, AxisFlag::Width);
        }
    );

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

    addInputHandler(
        DialogInputs::Length, [this]() {
            update(m_configuration.ctx<DialogLengthDividerCountInput>().control);
            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
        }
    );

    addInputHandler(
        DialogInputs::Width, [this]() {
            update(m_configuration.ctx<DialogLengthDividerCountInput>().control);
            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
        }
    );

    addInputHandler(
        DialogInputs::Height, [this]() {
            update(m_configuration.ctx<DialogLengthDividerCountInput>().control);
            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
        }
    );

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

auto CreateDialog::createJointTable(const adsk::core::Ptr<CommandInputs> &inputs) -> adsk::core::Ptr<TableCommandInput> {
    auto table = inputs->addTableCommandInput(
        "jointTableCommandInput", "Joints", 0, "1:1:1:1:1:1"
    );
    table->maximumVisibleRows(17);
    table->minimumVisibleRows(17);
    table->tablePresentationStyle(itemBorderTablePresentationStyle);

    auto const first_label    = table->commandInputs()->addTextBoxCommandInput("firstJointLabelInput", "First", "<b>First</b>", 1, true);
    auto const second_label   = table->commandInputs()->addTextBoxCommandInput("secondJointLabelInput", "Second", "<b>Second</b>", 1, true);
    auto const pattern_label  = table->commandInputs()->addTextBoxCommandInput("jointPatternLabelInput", "Pattern", "<b>Pattern</b>", 1, true);
    auto const type_label     = table->commandInputs()->addTextBoxCommandInput("jointPatternLabelInput", "Joint Type", "<b>Joint Type</b>", 1, true);
    auto const pos_label      = table->commandInputs()->addTextBoxCommandInput("posLabelInput", "Panel Offset", "<b>Panel Offset</b>", 1, true);
    auto const jos_label      = table->commandInputs()->addTextBoxCommandInput("josLabelInput", "Joint Offset", "<b>Joint Offset</b>", 1, true);
    auto const distance_label = table->commandInputs()->addTextBoxCommandInput("distanceLabelInput", "Distance", "<b>Distance</b>", 1, true);

    table->addCommandInput(first_label, 0, 0);
    table->addCommandInput(second_label, 0, 1);
    table->addCommandInput(pattern_label, 0, 2);
    table->addCommandInput(type_label, 0, 3);
    table->addCommandInput(pos_label, 0, 4);
    table->addCommandInput(jos_label, 0, 5);
    table->addCommandInput(distance_label, 0, 6);
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

    m_configuration.view<U, DialogPanelEnable, DialogPanelThickness>().each(
        [&](
            auto entity, auto const &dimension, auto const &enable, auto const &thickness
        ) {
            minimum += thickness.control->value() * enable.control->value();
        }
    );

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

        m_configuration.view<DialogPanelThickness>().each(
            [&](
                auto const &thickness
            ) {
                kerf_ok = kerf_ok && (thickness.control->value() > kerf);
            }
        );

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

        m_configuration.view<DialogPanelThickness>().each(
            [&](
                auto const &thickness
            ) {
                finger_ok = finger_ok && (thickness.control->value() < finger_width);
            }
        );

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

        m_configuration.view<DialogPanelEnable>().each(
            [&](
                auto const &enable
            ) {
                thickness_count += enable.control->value();
            }
        );

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

void CreateDialog::updatePanelOrientations() {
    addPanelOrientation<LengthOrientation, AxisFlag::Length>(m_configuration);
    addPanelOrientation<WidthOrientation, AxisFlag::Width>(m_configuration);
    addPanelOrientation<HeightOrientation, AxisFlag::Height>(m_configuration);
}

void CreateDialog::createPanelDimensions() {
    auto orientation = m_configuration.ctx<DialogModelingOrientation>();

    auto top    = m_configuration.ctx<DialogTopPanel>().id;
    auto bottom = m_configuration.ctx<DialogBottomPanel>().id;
    auto left   = m_configuration.ctx<DialogLeftPanel>().id;
    auto right  = m_configuration.ctx<DialogRightPanel>().id;
    auto front  = m_configuration.ctx<DialogFrontPanel>().id;
    auto back   = m_configuration.ctx<DialogBackPanel>().id;

    PLOG_DEBUG << "Top Panel ID: " + std::to_string((int) top);
    PLOG_DEBUG << "Bottom Panel ID: " + std::to_string((int) bottom);
    PLOG_DEBUG << "Left Panel ID: " + std::to_string((int) left);
    PLOG_DEBUG << "Right Panel ID: " + std::to_string((int) right);
    PLOG_DEBUG << "Front Panel ID: " + std::to_string((int) front);
    PLOG_DEBUG << "Back Panel ID: " + std::to_string((int) back);

    addPanelDimensions<DialogLengthInput, DialogWidthInput, DialogHeightInput>(top);
    addPanelDimensions<DialogLengthInput, DialogWidthInput, DialogBottomThickness>(bottom);
    addPanelDimensions<DialogLeftThickness, DialogWidthInput, DialogHeightInput>(left);
    addPanelDimensions<DialogLengthInput, DialogWidthInput, DialogHeightInput>(right);

    if (orientation.value == YUpModelingOrientation) {
        addPanelDimensions<DialogLengthInput, DialogWidthInput, DialogHeightInput>(front);
        addPanelDimensions<DialogLengthInput, DialogBackThickness, DialogHeightInput>(back);
    } else {
        addPanelDimensions<DialogLengthInput, DialogFrontThickness, DialogHeightInput>(front);
        addPanelDimensions<DialogLengthInput, DialogWidthInput, DialogHeightInput>(back);
    }

    addMaxOffset<DialogHeightInput>(top);
    addMaxOffset<DialogHeightInput>(bottom);
    addMaxOffset<DialogLengthInput>(left);
    addMaxOffset<DialogLengthInput>(right);
    addMaxOffset<DialogWidthInput>(front);
    addMaxOffset<DialogWidthInput>(back);
}

template<class L, class W, class H>
void CreateDialog::addPanelDimensions(entt::entity entity) {
    auto length = m_configuration.ctx<L>().control;
    auto width  = m_configuration.ctx<W>().control;
    auto height = m_configuration.ctx<H>().control;

    PLOG_DEBUG << "Panel dimensions: " << length->value() << ", " << width->value() << ", " << height->value();
    m_configuration.emplace<PanelDimensionInputs>(entity, length, width, height);
}


template<class M>
void CreateDialog::addMaxOffset(entt::entity entity) {
    auto offset = m_configuration.ctx<M>().control;

    m_configuration.emplace<MaxOffsetInput>(entity, offset);
}

void CreateDialog::projectPlanes() {
    PLOG_DEBUG << "CreateDialog::projectPlanes";
    projectLengthPlane();
    projectWidthPlane();
    projectHeightPlane();
}

void CreateDialog::projectLengthPlane() {
    PLOG_DEBUG << "CreateDialog::projectLengthPlane";

    m_configuration.view<DialogPanelPlanes, LengthOrientation, DialogPanelThickness, PanelDimensionInputs, DialogPanel>().each(
        [this](
            auto &planes, auto const &orientation, auto const &thickness, auto const &dimensions, auto const &panel
        ) {
            PLOG_DEBUG << "Thickness = " << thickness.control->value();
            PLOG_DEBUG << panel.name << " dimensions: " << dimensions.width->value() << ", " << dimensions.height->value();
            planes.length.max_x = round(dimensions.width->value() * 100000) / 100000;
            planes.length.max_y = round(dimensions.height->value() * 100000) / 100000;
            planes.length.min_x = 0;
            planes.length.min_y = 0;
            PLOG_DEBUG << panel.name << " length plane: (" << planes.length.min_x << ", " << planes.length.min_y << ") to (" << planes.length.max_x << ", "
                       << planes.length.max_y << ")";
        }
    );

    m_configuration.view<DialogPanelPlanes, WidthOrientation, DialogPanelThickness, PanelDimensionInputs, DialogPanel>().each(
        [this](
            auto &planes, auto const &orientation, auto const &thickness, auto const &dimensions, auto const &panel
        ) {
            planes.length.max_x = round(dimensions.width->value() * 100000) / 100000;
            planes.length.max_y = round(dimensions.height->value() * 100000) / 100000;
            planes.length.min_x = round((planes.length.max_x - thickness.control->value()) * 100000) / 100000;
            planes.length.min_y = 0;
            PLOG_DEBUG << panel.name << " length plane: (" << planes.length.min_x << ", " << planes.length.min_y << ") to (" << planes.length.max_x << ", "
                       << planes.length.max_y << ")";
        }
    );

    m_configuration.view<DialogPanelPlanes, HeightOrientation, DialogPanelThickness, PanelDimensionInputs, DialogPanel>().each(
        [this](
            auto &planes, auto const &orientation, auto const &thickness, auto const &dimensions, auto const &panel
        ) {
            planes.length.max_x = round(dimensions.width->value() * 100000) / 100000;
            planes.length.max_y = round(dimensions.height->value() * 100000) / 100000;
            planes.length.min_x = 0;
            planes.length.min_y = round((planes.length.max_y - thickness.control->value()) * 100000) / 100000;
            PLOG_DEBUG << panel.name << " length plane: (" << planes.length.min_x << ", " << planes.length.min_y << ") to (" << planes.length.max_x << ", "
                       << planes.length.max_y << ")";
        }
    );
}

void CreateDialog::projectWidthPlane() {
    PLOG_DEBUG << "CreateDialog::projectWidthPlane";

    m_configuration.view<DialogPanelPlanes, LengthOrientation, DialogPanelThickness, PanelDimensionInputs, DialogPanel>().each(
        [](
            auto &planes, auto const &orientation, auto const &thickness, auto const &dimensions, auto const &panel
        ) {
            planes.width.max_x = round(dimensions.length->value() * 100000) / 100000;
            planes.width.max_y = round(dimensions.height->value() * 100000) / 100000;
            planes.width.min_x = round((planes.width.max_x - thickness.control->value()) * 100000) / 100000;
            planes.width.min_y = 0;
            PLOG_DEBUG << panel.name << " width plane: (" << planes.width.min_x << ", " << planes.width.min_y << ") to (" << planes.width.max_x << ", "
                       << planes.width.max_y << ")";
        }
    );

    m_configuration.view<DialogPanelPlanes, WidthOrientation, DialogPanelThickness, PanelDimensionInputs, DialogPanel>().each(
        [](
            auto &planes, auto const &orientation, auto const &thickness, auto const &dimensions, auto const &panel
        ) {
            planes.width.max_x = round(dimensions.length->value() * 100000) / 100000;
            planes.width.max_y = round(dimensions.height->value() * 100000) / 100000;
            planes.width.min_x = 0;
            planes.width.min_y = 0;
            PLOG_DEBUG << panel.name << " width plane: (" << planes.width.min_x << ", " << planes.width.min_y << ") to (" << planes.width.max_x << ", "
                       << planes.width.max_y << ")";
        }
    );

    m_configuration.view<DialogPanelPlanes, HeightOrientation, DialogPanelThickness, PanelDimensionInputs, DialogPanel>().each(
        [](
            auto &planes, auto const &orientation, auto const &thickness, auto const &dimensions, auto const &panel
        ) {
            planes.width.max_x = round(dimensions.length->value() * 100000) / 100000;
            planes.width.max_y = round(dimensions.height->value() * 100000) / 100000;
            planes.width.min_x = 0;
            planes.width.min_y = round((planes.width.max_y - thickness.control->value()) * 100000) / 100000;
            PLOG_DEBUG << panel.name << " width plane: (" << planes.width.min_x << ", " << planes.width.min_y << ") to (" << planes.width.max_x << ", "
                       << planes.width.max_y << ")";
        }
    );

}

void CreateDialog::projectHeightPlane() {
    PLOG_DEBUG << "CreateDialog::projectHeightPlane";

    m_configuration.view<DialogPanelPlanes, LengthOrientation, DialogPanelThickness, PanelDimensionInputs, DialogPanel>().each(
        [](
            auto &planes, auto const &orientation, auto const &thickness, auto const &dimensions, auto const &panel
        ) {
            planes.height.max_x = round(dimensions.length->value() * 100000) / 100000;
            planes.height.max_y = round(dimensions.width->value() * 100000) / 100000;
            planes.height.min_x = round((planes.height.max_x - thickness.control->value()) * 100000) / 100000;
            planes.height.min_y = 0;
            PLOG_DEBUG << panel.name << " height plane: (" << planes.height.min_x << ", " << planes.height.min_y << ") to (" << planes.height.max_x << ", "
                       << planes.height.max_y << ")";
        }
    );

    m_configuration.view<DialogPanelPlanes, WidthOrientation, DialogPanelThickness, PanelDimensionInputs, DialogPanel>().each(
        [](
            auto &planes, auto const &orientation, auto const &thickness, auto const &dimensions, auto const &panel
        ) {
            planes.height.max_x = round(dimensions.length->value() * 100000) / 100000;
            planes.height.max_y = round(dimensions.width->value() * 100000) / 100000;
            planes.height.min_x = 0;
            planes.height.min_y = round((planes.height.max_y - thickness.control->value()) * 100000) / 100000;
            PLOG_DEBUG << panel.name << " height plane: (" << planes.height.min_x << ", " << planes.height.min_y << ") to (" << planes.height.max_x << ", "
                       << planes.height.max_y << ")";
        }
    );

    m_configuration.view<DialogPanelPlanes, HeightOrientation, DialogPanelThickness, PanelDimensionInputs, DialogPanel>().each(
        [](
            auto &planes, auto const &orientation, auto const &thickness, auto const &dimensions, auto const &panel
        ) {
            planes.height.max_x = round(dimensions.length->value() * 100000) / 100000;
            planes.height.max_y = round(dimensions.width->value() * 100000) / 100000;
            planes.height.min_x = 0;
            planes.height.min_y = 0;
            PLOG_DEBUG << panel.name << " height plane: (" << planes.height.min_x << ", " << planes.height.min_y << ") to (" << planes.height.max_x << ", "
                       << planes.height.max_y << ")";
        }
    );
}

void CreateDialog::findJoints() {
    auto first_entities = std::map<entt::entity, std::vector<entt::entity>>{};
    auto second_entities = std::map<entt::entity, std::vector<entt::entity>>{};
    m_configuration.set<DialogJointIndex>(first_entities, second_entities);

    m_configuration.view<DialogPanel, DialogPanelPlanes>().each(
        [this](
            auto entity, auto const &panel, auto const &planes
        ) {
            auto const name         = panel.name;
            auto const orientation  = panel.orientation;
            auto const& length      = planes.length;
            auto const& width       = planes.width;
            auto const& height      = planes.height;

            PLOG_DEBUG << name << " length plane:  (" << planes.length.min_x << ", " << planes.length.min_y << ") to (" << planes.length.max_x << ", " << planes.length.max_y << ")";
            PLOG_DEBUG << name << " width plane :  (" << planes.width.min_x << ", " << planes.width.min_y << ") to (" << planes.width.max_x << ", " << planes.width.max_y << ")";
            PLOG_DEBUG << name << " height plane:  (" << planes.height.min_x << ", " << planes.height.min_y << ") to (" << planes.height.max_x << ", " << planes.height.max_y << ")";

            findSecondaryPanels({entity, panel, planes});
        }
    );
}

void CreateDialog::findSecondaryPanels(
    DialogPanelJoint first
) {
    auto &index = m_configuration.ctx<DialogJointIndex>();
    auto &finger_mode = m_configuration.ctx<DialogFingerMode>();
    auto &finger_width = m_configuration.ctx<DialogFingerWidthInput>();

    m_configuration.view<DialogPanel, DialogPanelPlanes>().each(
        [&, this](
            auto entity, auto const &panel, auto const &second_planes
        ) {
            auto second = DialogPanelJoint{entity, panel, second_planes};

            if (first.panel.orientation == second.panel.orientation) return;

            auto first_result = detectPanelCollision(first, second);
            auto enabled = (first_result.collision_detected && first_result.first_is_primary);

            if (!enabled) return ;

            auto second_result = detectPanelCollision(second, first);

            auto joint_entity = m_configuration.create();
            m_configuration.emplace<Enabled>(joint_entity, enabled);
            m_configuration.emplace<DialogPanelCollisionData>(joint_entity, first_result.data, second_result.data);
            m_configuration.emplace<DialogFirstPlanes>(joint_entity, first.planes);
            m_configuration.emplace<DialogSecondPlanes>(joint_entity, second.planes);
            m_configuration.emplace<PanelPosition>(joint_entity, first_result.position);
            m_configuration.emplace<JointPosition>(joint_entity, second_result.position);
            m_configuration.emplace<DialogFingerMode>(joint_entity, finger_mode);
            m_configuration.emplace<FingerPatternType>(joint_entity);
            m_configuration.emplace<JointPattern>(joint_entity);
            m_configuration.emplace<entities::JointDirections>(joint_entity);
            m_configuration.emplace<FingerWidthInput>(joint_entity, finger_width.control);
            m_configuration.emplace<FingerWidth>(joint_entity);

            auto panels = DialogPanels{first.entity, second.entity};
            m_configuration.emplace<DialogPanels>(joint_entity, panels);

            PLOG_DEBUG << "Adding " << second.panel.name << " to list of second entities for " << first.panel.name;
            index.first_panels[first.entity].emplace_back(joint_entity);
            index.second_panels[second.entity].emplace_back(joint_entity);

            auto joint = DialogJoints{first, second};
            m_configuration.emplace<DialogJoints>(joint_entity, joint);
            m_configuration.emplace<DialogJointPattern>(joint_entity, first_result.pattern);
        }
    );
}

auto CreateDialog::detectPanelCollision(const DialogPanelJoint &first, const DialogPanelJoint &second) -> DialogPanelCollisionPair {
    auto const & first_orientation = first.panel.orientation;
    auto const& first_length       = first.planes.length;
    auto const& first_width        = first.planes.width;
    auto const& first_height       = first.planes.height;

    auto const& orientation        = second.panel.orientation;
    auto const& second_length      = second.planes.length;
    auto const& second_width       = second.planes.width;
    auto const& second_height      = second.planes.height;

    auto length_overlaps = (first_length.min_x <= second_length.max_x) && (first_length.max_x >= second_length.min_x)
                           && (first_length.max_y >= second_length.min_y) && (first_length.min_y <= second_length.max_y);

    PLOG_DEBUG << second.panel.name << " length plane:  (" << second_length.min_x << ", " << second_length.min_y << ") to (" << second_length.max_x << ", "
               << second_length.max_y << ")";
    PLOG_DEBUG << second.panel.name << " width plane :  (" << second_width.min_x << ", " << second_width.min_y << ") to (" << second_width.max_x << ", "
               << second_width.max_y << ")";
    PLOG_DEBUG << second.panel.name << " height plane:  (" << second_height.min_x << ", " << second_height.min_y << ") to (" << second_height.max_x << ", "
               << second_height.max_y << ")";

    auto width_overlaps = (first_width.min_x <= second_width.max_x) && (first_width.max_x >= second_width.min_x)
                          && (first_width.max_y >= second_width.min_y) && (first_width.min_y <= second_width.max_y);

    auto height_overlaps = (first_height.min_x <= second_height.max_x) && (first_height.max_x >= second_height.min_x)
                           && (first_height.max_y >= second_height.min_y) && (first_height.min_y <= second_height.max_y);

    auto collision_detected = length_overlaps && width_overlaps && height_overlaps;

    PLOG_DEBUG << first.panel.name << "(" << first_length.min_x << "," << first_length.min_y << ") panel overlaps with " << second.panel.name << "("
               << second_length.min_x << ","
               << second_length.min_y << ") panel.";

    auto const length_width_joint = first_orientation == AxisFlag::Length && orientation == AxisFlag::Width;
    auto const length_width_pos   = length_width_joint * (second_length.min_x - first_length.min_x);
    auto const length_width_jos   = length_width_joint * (second_length.min_y - first_length.min_y);
    auto const length_width_jd    = length_width_joint * (first_length.max_y - second_length.min_y);
    auto const length_width_outside = (collision_detected && length_width_joint) && (length_width_pos == 0 || (second_length.max_x >= first_length.min_x && second_length.min_x <= first_length.max_x));

    auto const length_height_joint = first_orientation == AxisFlag::Length && orientation == AxisFlag::Height;
    auto const length_height_pos   = length_height_joint * (second_length.min_y - first_length.min_y);
    auto const length_height_jos   = length_height_joint * (second_length.min_x - first_length.min_x);
    auto const length_height_jd    = length_height_joint * (first_length.max_x - second_length.min_x);
    auto const length_height_outside = (collision_detected && length_height_joint) && (length_height_pos == 0 || (second_length.max_y >= first_length.min_y && second_length.min_y <= first_length.max_y));

    auto const width_length_joint = first_orientation == AxisFlag::Width && orientation == AxisFlag::Length;
    auto const width_length_pos   = width_length_joint * (second_width.min_x - first_width.min_x);
    auto const width_length_jos   = width_length_joint * (second_width.min_y - first_width.min_y);
    auto const width_length_jd    = width_length_joint * (first_width.max_y - second_width.min_y);
    auto const width_length_outside = (collision_detected && width_length_joint) && (width_length_pos == 0 || (second_width.max_x >= first_width.min_x && second_width.min_x <= first_width.max_x));

    const auto width_height_joint = first_orientation == AxisFlag::Width && orientation == AxisFlag::Height;
    auto const width_height_pos   = width_height_joint * (second_width.min_y - first_width.min_y);
    auto const width_height_jos   = width_height_joint * (second_width.min_x - first_width.min_x);
    auto const width_height_jd    = width_height_joint * (first_width.max_x - second_width.min_x);
    auto const width_height_outside = (collision_detected && width_height_joint) && (width_height_pos == 0 || (second_width.max_y >= first_width.min_y && second_width.min_y <= first_width.max_y));

    const auto height_width_joint = first_orientation == AxisFlag::Height && orientation == AxisFlag::Width;
    auto const height_width_pos   = height_width_joint * (second_height.min_y - first_height.min_y);
    auto const height_width_jos   = height_width_joint * (second_height.min_x - first_height.min_x);
    auto const height_width_jd    = height_width_joint * (first_height.max_x - second_height.min_x);
    auto const height_width_outside = (collision_detected && height_width_joint) && (height_width_pos == 0 || (second_height.max_y >= first_height.min_y && second_height.min_y <= first_height.max_y));

    const auto height_length_joint = first_orientation == AxisFlag::Height && orientation == AxisFlag::Length;
    auto const height_length_pos   = height_length_joint * (second_height.min_x - first_height.min_x);
    auto const height_length_jos   = height_length_joint * (second_height.min_y - first_height.min_y);
    auto const height_length_jd    = height_length_joint * (first_height.max_y - second_height.min_y);
    auto const height_length_outside = (collision_detected && height_length_joint) && (height_length_pos == 0 || (second_height.max_x >= first_height.min_x && second_height.min_x <= first_height.max_x));

    auto panel_offset     = std::max({length_width_pos, length_height_pos, width_length_pos, width_height_pos, height_width_pos, height_length_pos});
    auto joint_offset     = std::max({length_width_jos, length_height_jos, width_length_jos, width_height_jos, height_width_jos, height_length_jos});
    auto joint_distance   = std::max({length_width_jd, length_height_jd, width_length_jd, width_height_jd, height_width_jd, height_length_jd});
    auto first_is_primary = first.panel.priority < second.panel.priority;
    auto ranking_str      = first_is_primary ? " is primary." : " is secondary.";
    auto joint_type       = static_cast<DialogJointPatternType>((int) (first.panel.priority > second.panel.priority));

    auto is_outside = (length_width_outside || length_height_outside || width_length_outside || width_height_outside || height_length_outside || height_width_outside);
    PLOG_DEBUG << second.panel.name << " panel offset from " << first.panel.name << " is " << panel_offset;
    PLOG_DEBUG << second.panel.name << " joint offset from " << first.panel.name << " is " << joint_offset;
    PLOG_DEBUG << second.panel.name << " joint distance for " << first.panel.name << " is " << joint_distance;
    PLOG_DEBUG << first.panel.name << ranking_str;
    PLOG_DEBUG << "Primary is outside == " << is_outside;

    return {
        collision_detected,
        first_is_primary,
        static_cast<Position>((int)is_outside),
        joint_type,
        panel_offset,
        joint_offset,
        joint_distance
    };
}

void CreateDialog::populateJointTable(Ptr<TableCommandInput> &table) {

    auto inputs  = table->commandInputs();
    auto row_num = 1;

    m_configuration.view<entities::DialogJoints, entities::DialogJointPattern, entities::DialogPanelCollisionData>().each(
        [&, this](
            auto entity, auto &joints, auto &pattern, auto &data
        ) {
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

            auto pos      = inputs->addTextBoxCommandInput("jointRowPanelOffset" + row_str, pos_str, pos_str, 1, true);
            auto jos      = inputs->addTextBoxCommandInput("jointRowJointOffset" + row_str, jos_str, jos_str, 1, true);
            auto distance = inputs->addTextBoxCommandInput("jointRowJointDistance" + row_str, dist_str, dist_str, 1, true);

            table->addCommandInput(first, row_num, 0);
            table->addCommandInput(second, row_num, 1);
            table->addCommandInput(dropdown, row_num, 2);
            table->addCommandInput(type_dropdown, row_num, 3);
            table->addCommandInput(pos, row_num, 4);
            table->addCommandInput(jos, row_num, 5);
            table->addCommandInput(distance, row_num, 6);

            m_configuration.emplace<DialogJointPanelOffsetInput>(entity, pos);
            m_configuration.emplace<DialogJointJointOffsetInput>(entity, jos);
            m_configuration.emplace<DialogJointDistanceOffsetInput>(entity, distance);

            row_num += 1;
        }
    );

    addCollisionHandler(DialogInputs::Length);
    addCollisionHandler(DialogInputs::Width);
    addCollisionHandler(DialogInputs::Height);
    addCollisionHandler(DialogInputs::Thickness);

}

void CreateDialog::addCollisionHandler(DialogInputs reference) {
    auto handler = [this]() {
        projectPlanes();
        updateJointPlanes();

        m_configuration.view<DialogFirstPlanes, DialogJoints>().each([this](
            auto entity, auto const first, auto &joints
        ){
            joints.first.planes = first.planes;
        });
        m_configuration.view<DialogSecondPlanes, DialogJoints>().each([this](
            auto entity, auto const second, auto &joints
        ){
            joints.second.planes = second.planes;
        });

        updateJoints();

        m_configuration.view<DialogPanelCollisionData, DialogJointPanelOffsetInput, DialogJointJointOffsetInput, DialogJointDistanceOffsetInput>().each(
            [](
                auto const &data, auto &panel_offset, auto &joint_offset, auto &distance
            ) {
                auto pos_val  = data.first.panel_offset == 0 ? 0 : data.first.panel_offset;
                auto jos_val  = data.first.joint_offset == 0 ? 0 : data.first.joint_offset;
                auto dist_val = data.first.distance == 0 ? 0 : data.first.distance;

                auto pos_str  = std::to_string(pos_val);
                auto jos_str  = std::to_string(jos_val);
                auto dist_str = std::to_string(dist_val);

                PLOG_DEBUG << "Updated panel offset: " << pos_str;
                PLOG_DEBUG << "Updated joint offset: " << jos_str;
                PLOG_DEBUG << "Updated joint distance: " << dist_str;

                panel_offset.control->formattedText(pos_str);
                joint_offset.control->formattedText(jos_str);
                distance.control->formattedText(dist_str);
            }
        );
    };

    addInputHandler(reference, handler);
}

void CreateDialog::updateJointPlanes() {

    auto index = m_configuration.ctx<DialogJointIndex>();

    m_configuration.view<DialogPanelPlanes>().each([&, this](
        auto parent_entity, auto const &planes
    ){
        for (auto const& entity: index.first_panels[parent_entity]) {
            m_configuration.replace<DialogFirstPlanes>(entity, planes);
        }
        for (auto const& entity: index.second_panels[parent_entity]) {
            m_configuration.replace<DialogSecondPlanes>(entity, planes);
        }
    });
}

void CreateDialog::updateJoints() {
    m_configuration.view<Enabled, DialogPanelCollisionData, DialogJoints>().each([this](
        auto &enabled, auto &collision, auto &joint
    ){
        auto first_result = detectPanelCollision(joint.first, joint.second);
        enabled.value = (first_result.collision_detected && first_result.first_is_primary);
        collision.first.panel_offset = first_result.data.panel_offset;
        collision.first.joint_offset = first_result.data.joint_offset;
        collision.first.distance = first_result.data.distance;

        auto second_result = detectPanelCollision(joint.second, joint.first);
        collision.second.panel_offset = second_result.data.panel_offset;
        collision.second.joint_offset = second_result.data.joint_offset;
        collision.second.distance = second_result.data.distance;
    });
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

    postUpdate();

    return true;
}

void CreateDialog::postUpdate() {
    PLOG_DEBUG << "Running post-update handlers";

    m_configuration.view<FingerPatternType, DialogFingerMode>().each([](
        auto entity, auto& pattern, auto const &finger_mode
    ){
        pattern.value = static_cast<FingerMode>(finger_mode.control->selectedItem()->index());
        PLOG_DEBUG << "Updating FingerPattern Type for entity " << (int)entity << " == " << (int)pattern.value;
    });

    m_configuration.view<FingerWidth, FingerWidthInput>().each([](
        FingerWidth &width, FingerWidthInput const& input
    ){
        width.value = input.control->value();
    });

    m_configuration.view<JointPattern, DialogJointPatternInput>().each([](
        auto entity, auto& pattern, auto const &input
    ){
        pattern.value = static_cast<JointPatternType>((int)input.control->selectedItem()->index());
        PLOG_DEBUG << "Updating JointPattern for entity " << (int)entity << " == " << (int)pattern.value;
    });

    m_configuration.view<entities::JointDirections, DialogJointDirectionInputs>().each([](
        auto entity, auto& directions, auto const &input
    ){
        auto result = (bool)input.first.control->selectedItem()->index();
        directions.first.value = static_cast<JointDirectionType>(result);
        directions.second.value = static_cast<JointDirectionType>(!result);
        PLOG_DEBUG << "Updating Joint Directions for entity " << (int)entity << ": first(" << (int)directions.first.value << "), second(" << (int)directions.second.value << ")";
    });
}

void CreateDialog::initializePanels() {

    m_panel_registry.clear();

    auto first_index = std::map<entt::entity, std::vector<entt::entity>>{};
    auto second_index = std::map<entt::entity, std::vector<entt::entity>>{};

    auto kerf           = m_configuration.ctx<DialogKerfInput>().control;
    auto length_divider = m_configuration.ctx<DialogLengthDividerJointInput>().control;
    auto width_divider  = m_configuration.ctx<DialogWidthDividerJointInput>().control;

    m_configuration.view<Enabled, FingerPatternType, FingerWidth, DialogJoints, DialogPanelCollisionData, DialogPanels, PanelPosition, JointPosition, JointPattern, entities::JointDirections>().each([&, this](
        auto entity, auto const &enabled, auto const &finger_mode, auto const &finger_width, auto const &joints, auto const &collision_data, auto const &panels, auto const &panel_position, auto const &joint_position, auto const &pattern_type, auto const &directions
    ){
        auto create_panel = [&, this](
            const DialogPanelJoint& joint, const entities::JointDirection& direction, const DialogPanelJointData collision, const entt::entity& second_panel
        ) {
            auto panel_offset = collision.panel_offset;
            auto joint_offset = collision.joint_offset;
            auto joint_distance = collision.distance;

            auto panel = m_panel_registry.create();
            m_panel_registry.emplace<KerfInput>(panel, kerf);
            m_panel_registry.emplace<FingerPatternType>(panel, finger_mode);
            m_panel_registry.emplace<FingerWidth>(panel, finger_width);

            m_panel_registry.emplace<JointPattern>(panel, pattern_type);
            m_panel_registry.emplace<JointDirection>(panel, direction);
            m_panel_registry.emplace<JointPanelOffset>(panel, panel_offset);
            m_panel_registry.emplace<JointPatternDistance>(panel, joint_distance);
            m_panel_registry.emplace<JointPosition>(panel, joint_position.value);
            m_panel_registry.emplace<PanelPosition>(panel, panel_position.value);
            m_panel_registry.emplace<JointProfile>(
                panel, Position::Outside, Position::Outside, JointDirectionType::Normal, JointPatternType::BoxJoint, FingerMode::Automatic, 0, 0.0, 0.0, 0.0, 0.0, AxisFlag::Length, AxisFlag::Length
            );
            m_panel_registry.emplace<JointPatternPosition>(
                panel, Position::Outside, AxisFlag::Length, JointPatternType::BoxJoint, AxisFlag::Length, Position::Outside
            );

            m_panel_registry.emplace<PanelOffset>(panel);
            m_panel_registry.emplace<Dimensions>(panel);
            m_panel_registry.emplace<EndReferencePoint>(panel);
            m_panel_registry.emplace<ExtrusionDistance>(panel);
            m_panel_registry.emplace<MaxOffset>(panel);
            m_panel_registry.emplace<PanelProfile>(panel);
            m_panel_registry.emplace<StartReferencePoint>(panel);

            PLOG_DEBUG << "Adding panel registry entity for " << joint.panel.name;
            first_index[joint.entity].emplace_back(panel);
            PLOG_DEBUG << joint.panel.name << " now has " << first_index[joint.entity].size() << " elements.";
            second_index[second_panel].emplace_back(panel);
        };

        create_panel(joints.first, directions.first, collision_data.first, joints.second.entity);
        create_panel(joints.second, directions.second, collision_data.second, joints.first.entity);
    });

    m_configuration.view<DialogPanelInputs, DialogPanel, PanelDimensionInputs, DialogPanelThickness, MaxOffsetInput>().each([&, this](
        auto entity, auto const &inputs, auto const &panel_data, auto const &dimensions, auto const &thickness, auto const &max_offset
    ){
        auto first_panels = first_index[entity];
        auto second_panels = second_index[entity];

        auto parent_panel = m_panel_registry.create();
        m_panel_registry.emplace<Panel>(parent_panel, panel_data.name, panel_data.priority, panel_data.orientation);
        m_panel_registry.emplace<ChildPanels>(parent_panel, first_panels);

        for (auto const &panel: first_panels) {
            PLOG_DEBUG << "Adding enable, panel and dimension data to panel " << panel_data.name;
            m_panel_registry.emplace<EnableInput>(panel, inputs.enabled);
            m_panel_registry.emplace<Panel>(panel, panel_data.name, panel_data.priority, panel_data.orientation);
            m_panel_registry.emplace<DimensionsInputs>(panel, dimensions.length, dimensions.width, dimensions.height);
            m_panel_registry.emplace<ThicknessInput>(panel, inputs.thickness);
            m_panel_registry.emplace<MaxOffsetInput>(panel, max_offset);
            m_panel_registry.emplace<ParentPanel>(panel, parent_panel);
        }

        for (auto const &panel: second_panels) {
            PLOG_DEBUG << "Adding joint name for " << panel_data.name;
            m_panel_registry.emplace<JointEnabledInput>(panel, inputs.enabled);
            m_panel_registry.emplace<JointName>(panel, panel_data.name);
            m_panel_registry.emplace<JointOrientation>(panel, panel_data.orientation);
            m_panel_registry.emplace<JointThickness>(panel, thickness.control->value());
        }
    });

    m_panel_registry.view<JointName, Panel, JointDirection>().each([](
       auto const& joint, auto const& panel, auto const& direction
    ){
        PLOG_DEBUG << "Found " << panel.name << " with joint to " << joint.value << " with direction " << (int)direction.value;
    });

    m_panel_registry.view<JointProfile, JointDirection>().each([](
        auto entity, auto& profile, JointDirection const& direction
    ){
        PLOG_DEBUG << "Setting Joint Profile direction for " << (int)entity << " to " << (int)direction.value;
        profile.joint_direction = direction.value;
    });

    m_panel_registry.view<JointProfile, PanelPosition, JointPosition>().each([](
        auto entity, auto &profile, auto const &panel, auto const &joint
    ){
        PLOG_DEBUG << "Updating joint profile with panel and joint position";
        profile.panel_position = panel.value;
        profile.joint_position = joint.value;
    });

    m_panel_registry.view<JointProfile, Panel, JointOrientation>().each([this](
        auto entity, auto &profile, auto const &panel, auto const &joint
    ){
        PLOG_DEBUG << "Add orientation group for " << panel.name;
        profile.panel_orientation = panel.orientation;
        profile.joint_orientation = joint.axis;
        m_panel_registry.emplace<OrientationGroup>(entity, panel.orientation, joint.axis);
    });

    m_panel_registry.view<JointProfile, JointPattern>().each([this](
        auto entity, auto &profile, auto const &pattern
    ){
        profile.joint_type = pattern.value;
    });

    m_panel_registry.view<JointPatternPosition, PanelPosition, JointPosition>().each([](
       auto entity, JointPatternPosition &pattern, PanelPosition const &panel, JointPosition const &joint
    ){
        PLOG_DEBUG << "Updating joint pattern position with panel and joint position";
        pattern.panel_position = panel.value;
        pattern.joint_position = joint.value;
    });

    m_panel_registry.view<Panel>().each([this](
       auto entity, auto const &panel
    ){
        if (panel.orientation != AxisFlag::Length) return;
        PLOG_DEBUG << "Add length orientation for " << panel.name;
        m_panel_registry.emplace<LengthOrientation>(entity);
    });

    m_panel_registry.view<Panel>().each([this](
        auto entity, auto const &panel
    ){
        if (panel.orientation != AxisFlag::Width) return;
        PLOG_DEBUG << "Add width orientation for " << panel.name;
        m_panel_registry.emplace<WidthOrientation>(entity);
    });

    m_panel_registry.view<Panel>().each([this](
        auto entity, auto const &panel
    ){
        if (panel.orientation != AxisFlag::Height) return;
        PLOG_DEBUG << "Add height orientation for " << panel.name;
        m_panel_registry.emplace<HeightOrientation>(entity);
    });

    m_panel_registry.view<FingerPatternType>().each([this](
       auto entity, auto const &pattern
    ){
        if (pattern.value != FingerMode::Automatic) return;
        PLOG_DEBUG << "Add automatic finger pattern type.";
        m_panel_registry.emplace<AutomaticFingerPatternType>(entity);
    });

    m_panel_registry.view<FingerPatternType>().each([this](
       auto entity, auto const &pattern
    ){
        if (pattern.value != FingerMode::Constant) return;
        PLOG_DEBUG << "Add constant finger pattern type.";
        m_panel_registry.emplace<ConstantFingerPatternType>(entity);
    });

    m_panel_registry.view<FingerPatternType>().each([this](
       auto entity, auto const &pattern
    ){
        if (pattern.value != FingerMode::None) return;
        PLOG_DEBUG << "Add no finger pattern type.";
        m_panel_registry.emplace<NoFingerPatternType>(entity);
    });

    m_panel_registry.view<JointDirection>().each([this](
        auto entity, auto const &direction
    ){
        if (direction.value != JointDirectionType::Normal) return;
        PLOG_DEBUG << "Add normal joint direction.";
        m_panel_registry.emplace<NormalJointDirection>(entity);
    });

    m_panel_registry.view<JointDirection>().each([this](
        auto entity, auto const &direction
    ){
        if (direction.value != JointDirectionType::Inverted) return;
        PLOG_DEBUG << "Add inverse joint direction.";
        m_panel_registry.emplace<InverseJointDirection>(entity);
    });

    m_panel_registry.view<JointPattern>().each([this](
        auto entity, auto const &pattern
    ){
        if (pattern.value != JointPatternType::BoxJoint) return;
        PLOG_DEBUG << "Updating BoxJointPattern";
        m_panel_registry.emplace<BoxJointPattern>(entity);
    });

    m_panel_registry.view<JointPattern>().each([this](
        auto entity, auto const &pattern
    ){
        if (pattern.value != JointPatternType::LapJoint) return;
        PLOG_DEBUG << "Updating LapJointPattern";
        m_panel_registry.emplace<LapJointPattern>(entity);
    });

    m_panel_registry.view<JointPattern>().each([this](
        auto entity, auto const &pattern
    ){
        if (pattern.value != JointPatternType::Tenon) return;
        PLOG_DEBUG << "Updating TenonJointPattern";
        m_panel_registry.emplace<TenonJointPattern>(entity);
    });

    m_panel_registry.view<JointPattern>().each([this](
        auto entity, auto const &pattern
    ){
        if (pattern.value != JointPatternType::DoubleTenon) return;
        PLOG_DEBUG << "Updating DoubleTenontJointPattern";
        m_panel_registry.emplace<DoubleTenonJointPattern>(entity);
    });

    m_panel_registry.view<JointPattern>().each([this](
        auto entity, auto const &pattern
    ){
        if (pattern.value != JointPatternType::TripleTenon) return;
        PLOG_DEBUG << "Updating TripleTenonJointPattern";
        m_panel_registry.emplace<TripleTenonJointPattern>(entity);
    });

    m_panel_registry.view<JointPattern>().each([this](
        auto entity, auto const &pattern
    ){
        if (pattern.value != JointPatternType::QuadTenon) return;
        PLOG_DEBUG << "Updating QuadTenonJointPattern";
        m_panel_registry.emplace<QuadTenonJointPattern>(entity);
    });

    m_panel_registry.view<JointPattern>().each([this](
        auto entity, auto const &pattern
    ){
        if (pattern.value != JointPatternType::Trim) return;
        PLOG_DEBUG << "Updating TrimJointPattern";
        m_panel_registry.emplace<TrimJointPattern>(entity);
    });

    m_panel_registry.view<JointEnabledInput>().each([this](
        auto entity, auto const &enabled
    ){
        PLOG_DEBUG << "Updating JointEnabled value";
        m_panel_registry.emplace<JointEnabled>(entity, enabled.control->value());
    });

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
