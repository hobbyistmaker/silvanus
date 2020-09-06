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
#include "entities/FingerPattern.hpp"
#include "entities/JointDirection.hpp"
#include "entities/JointName.hpp"
#include "entities/Kerf.hpp"
#include "entities/OrientationTags.hpp"
#include "entities/OutsidePanel.hpp"
#include "entities/OverrideInput.hpp"
#include "entities/Panel.hpp"
#include "entities/PanelDimension.hpp"
#include "entities/Position.hpp"
#include "entities/StandardJoint.hpp"

#include <numeric>

#include "plog/Log.h"

using std::accumulate;
using std::all_of;
using std::get;
using std::vector;

using adsk::core::Ptr;
using adsk::core::Application;
using adsk::core::BoolValueCommandInput;
using adsk::core::CommandInputs;
using adsk::core::DefaultModelingOrientations;
using adsk::core::DropDownCommandInput;
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

    auto const dimensions = inputs->addTabCommandInput("dimensionsTabInput", "", "resources/dimensions");
    dimensions->tooltip("Dimensions");

    auto dimension_children = dimensions->children();

    createModelSelectionDropDown(dimension_children);
    createFingerModeSelectionDropDown(dimension_children);

    auto dimensions_group = createDimensionGroup(dimension_children);
    createPanelTable(dimensions_group->children());

    auto const dividers = inputs->addTabCommandInput("dividersTabInput", "", "resources/dividers");
    dividers->tooltip("Dividers");
    createDividerInputs(dividers->children());

    auto const joints = inputs->addTabCommandInput("panelJointsTabInput", "Joints");
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
}

void CreateDialog::addInputControl(DialogInputs reference, const adsk::core::Ptr<CommandInput>& input) {
    m_inputs[reference] = input->id();
}

void CreateDialog::addInputControl(DialogInputs reference, const adsk::core::Ptr<CommandInput>& input, const std::function<void()> &handler) {
    addInputControl(reference, input);
    m_handlers[input->id()].emplace_back(handler);
    m_inputs[reference] = input->id();
}

void CreateDialog::addInputControl(
    const DialogInputs reference,
    const adsk::core::Ptr<CommandInput> &input,
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
    createDividerOrientationsInput(inputs);
    createDividerJointDirectionInput(inputs);

    createLengthDividerInputs(inputs);
    createWidthDividerInputs(inputs);
    createHeightDividerInputs(inputs);
}

void CreateDialog::createHeightDividerInputs(const Ptr<CommandInputs> &inputs) {
    auto height_group    = inputs->addGroupCommandInput("heightDividerGroupInput", "Height Dividers");
    auto height_children = height_group->children();
    height_group->isVisible(false);
    m_configuration.set<DialogHeightDividerGroupInput>(height_group);

    auto height_divider_fb_outside_joint = height_children->addDropDownCommandInput(
        "heightDividerOutsideFBJointInput", "Front/Back Joints", TextListDropDownStyle
    );
    m_configuration.set<DialogHeightDividerFrontBackJointInput>(height_divider_fb_outside_joint);
    addJointTypes(height_divider_fb_outside_joint);
    addInputControl(
        DialogInputs::HeightDividerFBJointInput, height_divider_fb_outside_joint, [this]() {
            update(m_configuration.ctx<DialogLengthDividerCountInput>().control);
            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
            update(m_configuration.ctx<DialogHeightDividerCountInput>().control);
        }
    );

    auto height_divider_lr_outside_joint = height_children->addDropDownCommandInput(
        "heightDividerOutsideLRJointInput", "Left/Right Joints", TextListDropDownStyle
    );
    m_configuration.set<DialogHeightDividerLeftRightJointInput>(height_divider_lr_outside_joint);
    addJointTypes(height_divider_lr_outside_joint);
    addInputControl(
        DialogInputs::HeightDividerLRJointInput, height_divider_lr_outside_joint, [this]() {
            update(m_configuration.ctx<DialogLengthDividerCountInput>().control);
            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
            update(m_configuration.ctx<DialogHeightDividerCountInput>().control);
        }
    );

    auto height = adsk::core::Ptr<IntegerSpinnerCommandInput>{
        height_children->addIntegerSpinnerCommandInput(
            "heightDividerCommandInput", "(#) Height Dividers", 0, 25, 1, 0
        )
    };
    m_configuration.set<DialogHeightDividerCountInput>(height);

    addInputControl(
        DialogInputs::HeightDividerCount, height, [this]() {
            auto orientations = m_configuration.ctx<DialogDividerOrientationsInput>().control->selectedItem()->index();
            if (orientations == 0) return;

            auto const divider_height    = m_configuration.ctx<DialogHeightInput>().control->value();
            auto const divider_inverted = m_configuration.ctx<DialogDividerJointInput>().control->selectedItem()->index() == 0 ;

            auto old_view = m_configuration.view<HeightDivider>();
            m_configuration.destroy(old_view.begin(), old_view.end());

            auto static_view = m_configuration.view<HeightDividerJoint>();
            m_configuration.destroy(static_view.begin(), static_view.end());

            PLOG_DEBUG << "Updating Height Divider information";

            auto dividers = Dividers<HeightDivider, DialogHeightDividerCountInput>(m_configuration, m_app);
            dividers.create(AxisFlag::Height, "Height", divider_height, 5);
            dividers.addOrientation<HeightOrientation>();
            dividers.addMaxOffset<DialogHeightInput>();

            auto is_inverted = (orientations == 1 && divider_inverted) || (orientations == 2 && !divider_inverted);
            auto inside_direction = static_cast<JointDirectionType>(is_inverted);
            m_systems->updateCollisions();
            m_systems->findJoints<Panel, HeightDivider, HeightDividerJoint>();
            m_systems->updateJointPatternInputs<HeightDividerJoint, DialogHeightDividerFrontBackJointInput>(AxisFlag::Width);
            m_systems->updateJointPatternInputs<HeightDividerJoint, DialogHeightDividerLeftRightJointInput>(AxisFlag::Length);
            m_systems->updateJointDirection<HeightDividerJoint>(Position::Outside, Position::Inside, JointDirectionType::Normal);
            m_systems->updateJointDirection<HeightDividerJoint>(Position::Inside, Position::Inside, inside_direction);
        }
    );
}

void CreateDialog::createWidthDividerInputs(const Ptr<CommandInputs> &inputs) {
    auto width_group    = inputs->addGroupCommandInput("widthDividerGroupInput", "Width Dividers");
    auto width_children = width_group->children();
    m_configuration.set<DialogWidthDividerGroupInput>(width_group);

    auto width_divider_lr_outside_joint = width_children->addDropDownCommandInput(
        "widthDividerOutsideLRJointInput", "Left/Right Joints", TextListDropDownStyle
        );
    m_configuration.set<DialogWidthDividerLeftRightJointInput>(width_divider_lr_outside_joint);
    addJointTypes(width_divider_lr_outside_joint);
    addInputControl(
        DialogInputs::WidthDividerLRJointInput, width_divider_lr_outside_joint, [this]() {
            update(m_configuration.ctx<DialogLengthDividerCountInput>().control);
            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
            update(m_configuration.ctx<DialogHeightDividerCountInput>().control);
        }
    );

    auto width_divider_tb_outside_joint = width_children->addDropDownCommandInput(
        "widthDividerOutsideTBJointInput", "Top/Bottom Joints", TextListDropDownStyle
        );
    m_configuration.set<DialogWidthDividerTopBottomJointInput>(width_divider_tb_outside_joint);
    addJointTypes(width_divider_tb_outside_joint);
    addInputControl(
        DialogInputs::WidthDividerTBJointInput, width_divider_tb_outside_joint, [this]() {
            update(m_configuration.ctx<DialogLengthDividerCountInput>().control);
            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
            update(m_configuration.ctx<DialogHeightDividerCountInput>().control);
        }
    );

    auto width = adsk::core::Ptr<IntegerSpinnerCommandInput>{
        width_children->addIntegerSpinnerCommandInput(
            "widthDividerCommandInput", "(#) Width Dividers", 0, 25, 1, 0
        )
    };
    m_configuration.set<DialogWidthDividerCountInput>(width);

    addInputControl(
        DialogInputs::WidthDividerCount, width, [this]() {
            auto orientations = m_configuration.ctx<DialogDividerOrientationsInput>().control->selectedItem()->index();
            if (orientations == 1) return;

            auto const divider_length    = m_configuration.ctx<DialogWidthInput>().control->value();
            auto const divider_inverted = m_configuration.ctx<DialogDividerJointInput>().control->selectedItem()->index() == 1;

            auto old_view = m_configuration.view<WidthDivider>();
            m_configuration.destroy(old_view.begin(), old_view.end());

            auto static_view = m_configuration.view<WidthDividerJoint>();
            m_configuration.destroy(static_view.begin(), static_view.end());

            auto dividers = Dividers<WidthDivider, DialogWidthDividerCountInput>(m_configuration, m_app);
            dividers.create(AxisFlag::Width, "Width", divider_length, 4);
            dividers.addOrientation<WidthOrientation>();
            dividers.addMaxOffset<DialogWidthInput>();

            auto is_inverted = (orientations == 0 && !divider_inverted) || (orientations == 2 && divider_inverted);
            auto inside_direction = static_cast<JointDirectionType>(is_inverted);
            m_systems->updateCollisions();
            m_systems->findJoints<Panel, WidthDivider, WidthDividerJoint>(orientations == 2);
            m_systems->updateJointPatternInputs<WidthDividerJoint, DialogWidthDividerLeftRightJointInput>(AxisFlag::Length);
            m_systems->updateJointPatternInputs<WidthDividerJoint, DialogWidthDividerTopBottomJointInput>(AxisFlag::Height);
            m_systems->updateJointDirection<WidthDividerJoint>(Position::Outside, Position::Inside, JointDirectionType::Normal);
            m_systems->updateJointDirection<WidthDividerJoint>(Position::Inside, Position::Inside, inside_direction);
        }
    );
}

void CreateDialog::createLengthDividerInputs(const Ptr<CommandInputs> &inputs) {
    auto length_group    = inputs->addGroupCommandInput("lengthDividerGroupInput", "Length Dividers");
    auto length_children = length_group->children();
    m_configuration.set<DialogLengthDividerGroupInput>(length_group);

    auto length_divider_fb_outside_joint = length_children->addDropDownCommandInput(
        "lengthDividerOutsideFBJointInput", "Front/Back Joints", TextListDropDownStyle
        );
    m_configuration.set<DialogLengthDividerFrontBackJointInput>(length_divider_fb_outside_joint);
    addJointTypes(length_divider_fb_outside_joint);
    addInputControl(
        DialogInputs::LengthDividerFBJointInput, length_divider_fb_outside_joint, [this]() {
            update(m_configuration.ctx<DialogLengthDividerCountInput>().control);
            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
            update(m_configuration.ctx<DialogHeightDividerCountInput>().control);
        }
    );

    auto length_divider_tb_outside_joint = length_children->addDropDownCommandInput(
        "lengthDividerOutsideTBJointInput", "Top/Bottom Joints", TextListDropDownStyle
        );
    m_configuration.set<DialogLengthDividerTopBottomJointInput>(length_divider_tb_outside_joint);
    addJointTypes(length_divider_tb_outside_joint);
    addInputControl(
        DialogInputs::LengthDividerTBJointInput, length_divider_tb_outside_joint, [this]() {
            update(m_configuration.ctx<DialogLengthDividerCountInput>().control);
            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
            update(m_configuration.ctx<DialogHeightDividerCountInput>().control);
        }
    );

    auto length = adsk::core::Ptr<IntegerSpinnerCommandInput>{
        length_children->addIntegerSpinnerCommandInput(
            "lengthDividerCommandInput", "(#) Length Dividers", 0, 25, 1, 0
        )
    };
    m_configuration.set<DialogLengthDividerCountInput>(length);

    addInputControl(
        DialogInputs::LengthDividerCount, length, [this]() {
            auto orientations = m_configuration.ctx<DialogDividerOrientationsInput>().control->selectedItem()->index();
            if (orientations == 2) return;

            auto const divider_length   = m_configuration.ctx<DialogLengthInput>().control->value();
            auto const divider_inverted = m_configuration.ctx<DialogDividerJointInput>().control->selectedItem()->index() == 1;

            auto old_view = m_configuration.view<LengthDivider>();
            m_configuration.destroy(old_view.begin(), old_view.end());

            auto static_view = m_configuration.view<LengthDividerJoint>();
            m_configuration.destroy(static_view.begin(), static_view.end());

            PLOG_DEBUG << "Updating Length Divider information";

            auto dividers = Dividers<LengthDivider, DialogLengthDividerCountInput>(m_configuration, m_app);
            dividers.create(AxisFlag::Length, "Length", divider_length, 3);
            dividers.addOrientation<LengthOrientation>();
            dividers.addMaxOffset<DialogLengthInput>();

            auto is_inverted = (orientations == 0 && divider_inverted) || (orientations == 1 && !divider_inverted);
            auto inside_direction = static_cast<JointDirectionType>(is_inverted);
            m_systems->updateCollisions();
            m_systems->findJoints<Panel, LengthDivider, LengthDividerJoint>(true);
            m_systems->updateJointPatternInputs<LengthDividerJoint, DialogLengthDividerFrontBackJointInput>(AxisFlag::Width);
            m_systems->updateJointPatternInputs<LengthDividerJoint, DialogLengthDividerTopBottomJointInput>(AxisFlag::Height);
            m_systems->updateJointDirection<LengthDividerJoint>(Position::Outside, Position::Inside, JointDirectionType::Normal);
            m_systems->updateJointDirection<LengthDividerJoint>(Position::Inside, Position::Inside, inside_direction);
        }
    );
}

void CreateDialog::createDividerJointDirectionInput(const Ptr<CommandInputs> &inputs) {
    auto divider_joint = inputs->addDropDownCommandInput("dividerLapCommandInput", "Divider Joint", TextListDropDownStyle);
    m_configuration.set<DialogDividerJointInput>(divider_joint);
    auto const joint_items = divider_joint->listItems();
    joint_items->add("Normal", true);
    joint_items->add("Inverse", false);
    divider_joint->maxVisibleItems(2);

    addInputControl(
        DialogInputs::DividerLapInput, divider_joint, [this]() {
            update(m_configuration.ctx<DialogLengthDividerCountInput>().control);
            update(m_configuration.ctx<DialogWidthDividerCountInput>().control);
            update(m_configuration.ctx<DialogHeightDividerCountInput>().control);
        }
    );
}

void CreateDialog::createDividerOrientationsInput(const Ptr<CommandInputs> &inputs) {
    auto divider_orientations = inputs->addDropDownCommandInput("dividerOrientationCommandInput", "Divider Orientations", TextListDropDownStyle);
    m_configuration.set<DialogDividerOrientationsInput>(divider_orientations);
    auto const type_items = divider_orientations->listItems();
    type_items->add("Length & Width", true);
    type_items->add("Length & Height", false);
    type_items->add("Width & Height", false);
    divider_orientations->maxVisibleItems(3);

    addInputControl(
        DialogInputs::DividerOrientations, divider_orientations, [this]() {
            auto orientations = m_configuration.ctx<DialogDividerOrientationsInput>().control->selectedItem()->index();

            auto length_group_input = m_configuration.ctx<DialogLengthDividerGroupInput>().control;
            auto length_count_input = m_configuration.ctx<DialogLengthDividerCountInput>().control;
            auto width_group_input = m_configuration.ctx<DialogWidthDividerGroupInput>().control;
            auto width_count_input = m_configuration.ctx<DialogWidthDividerCountInput>().control;
            auto height_group_input = m_configuration.ctx<DialogHeightDividerGroupInput>().control;
            auto height_count_input = m_configuration.ctx<DialogHeightDividerCountInput>().control;

            auto selector = std::__1::map<int, std::function<void()>>{
                {0, [&, this](){
                    height_count_input->value(0);
                    update(height_count_input);
                }},
                {1, [&, this](){
                    width_count_input->value(0);
                    update(width_count_input);
                }},
                {2, [&, this](){
                    length_count_input->value(0);
                    update(length_count_input);
                }}
            };

            auto length_is_valid = orientations < 2;
            auto width_is_valid = orientations == 0 || orientations == 2;
            auto height_is_valid = orientations > 0;

            length_group_input->isVisible(length_is_valid);
            width_group_input->isVisible(width_is_valid);
            height_group_input->isVisible(height_is_valid);

            selector[orientations]();
        }
    );
}

auto CreateDialog::createStandardJointTable(const adsk::core::Ptr<CommandInputs> &inputs) -> adsk::core::Ptr<TableCommandInput> {
    auto joint_group = inputs->addGroupCommandInput("standardJointsGroupCommandInput", "Standard");
    auto group_children = joint_group->children();
    m_configuration.set<DialogStandardJointGroupInput>(joint_group);

    auto table = group_children->addTableCommandInput(
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
        addJointTypes(type_dropdown);

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

void CreateDialog::addJointTypes(Ptr<DropDownCommandInput> &dropdown) {
    auto const &items = dropdown->listItems();
    items->add("Box Joint", true);
    items->add("Lap Joint", false);
    items->add("Tenon", false);
    items->add("Double Tenon", false);
    items->add("Triple Tenon", false);
    items->add("Quad Tenon", false);
    items->add("Trim", false);
//            items->add("Pattern", false);
    items->add("None", false);
    dropdown->maxVisibleItems(7);
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
    m_systems->initializePanels(m_panel_registry);
}

