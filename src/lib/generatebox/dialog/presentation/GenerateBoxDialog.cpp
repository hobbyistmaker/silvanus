//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/22/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//

#include "presentation/GenerateBoxDialog.hpp"
#include "presentation/dividers.hpp"
#include "presentation/PanelDialogControls.hpp"
#include "presentation/validateDimensions.hpp"

#include "entities/EntitiesAll.hpp"

#include "entity_helpers.hpp"
#include "PanelConfigurationManager.hpp"

#include <plog/Log.h>
#include <Core/CoreAll.h>

#include <boost/algorithm/string.hpp>

using std::all_of;
using std::get;
using std::vector;

using adsk::core::Ptr;
using adsk::core::Application;
using adsk::core::BoolValueCommandInput;
using adsk::core::CommandInput;
using adsk::core::CommandInputs;
using adsk::core::DefaultModelingOrientations;
using adsk::core::DropDownCommandInput;
using adsk::core::FloatSpinnerCommandInput;
using adsk::core::GroupCommandInput;
using adsk::core::IntegerSpinnerCommandInput;
using adsk::core::TabCommandInput;
using adsk::core::TableCommandInput;
using adsk::core::TablePresentationStyles;
using adsk::core::TextListDropDownStyle;
using adsk::core::TextBoxCommandInput;
using adsk::fusion::Component;

using namespace silvanus::generatebox;
using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::dialog;

using silvanus::generatebox::entities::JointDirections;
using panelMap = std::map<silvanus::generatebox::entities::Panels, entt::entity>;

void GenerateBoxDialog::clear() {
    m_validators.clear();
    m_handlers.clear();
    m_results.clear();
    m_inputs.clear();
    m_ignore_updates.clear();
    m_configuration.clear();
    m_panel_registry.clear();
}

void GenerateBoxDialog::create(
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

    auto config_mgr = PanelConfigurationManager(m_configuration, is_metric);
    auto back   = config_mgr.initializePanel(m_back_row);
    auto front  = config_mgr.initializePanel(m_front_row);
    auto right  = config_mgr.initializePanel(m_right_row);
    auto left   = config_mgr.initializePanel(m_left_row);
    auto bottom = config_mgr.initializePanel(m_bottom_row);
    auto top    = config_mgr.initializePanel(m_top_row);

    auto const dimensions = inputs->addTabCommandInput("dimensionsTabInput", "", "resources/dimensions");
    dimensions->tooltip("Dimensions");

    auto dimension_children = dimensions->children();
    createModelSelectionDropDown(dimension_children);
    createFingerModeSelectionDropDown(dimension_children);

    auto dimensions_group = createDimensionGroup(dimension_children);

    createDimensionParameters();

    top.configure(maxHeightPanel);
    bottom.configure(minHeightPanel);
    left.configure(minLengthPanel);
    right.configure(maxLengthPanel);

    if (orientation == adsk::core::YUpModelingOrientation) {
        front.configure(maxWidthPanel);
        back.configure(minWidthPanel);
    } else {
        front.configure(minWidthPanel);
        back.configure(maxWidthPanel);
    }

    createPanelTable(dimensions_group->children(), config_mgr);

    auto const dividers = inputs->addTabCommandInput("dividersTabInput", "", "resources/dividers");
    dividers->tooltip("Dividers");
    createDividerInputs(dividers->children());

    createFollowThicknessValueHandlers(config_mgr);
    createOverrideEnableHandlers(config_mgr);
    createThicknessEnableHandlers(config_mgr);

    auto const joints = inputs->addTabCommandInput("panelJointsTabInput", "Joints");
    joints->tooltip("Panel Joints");
    auto joint_table = createStandardJointTable(joints->children());
//
//    auto const panels = inputs->addTabCommandInput("panelOffsetsTabInput", "Offsets");
//    panels->tooltipDescription("Define the offset from the length, width and height axes for each panel.");
//    auto panel_table = createPanelOffsetTable(panels->children());
//
    createPreviewTable(inputs);

    m_error = inputs->addTextBoxCommandInput("errorMessageCommandInput", "", "", 2, true);
    m_error->isVisible(false);

    m_ignore_updates.emplace_back(m_error->id());

    addMinimumAxisDimensionChecks();
    addMaximumKerfCheck();
    addMinimumFingerWidthCheck();
    addMinimumPanelCountCheck();

    m_systems->updateCollisions();
    m_systems->findJoints<OutsidePanel, StandardJoint>();

    populateJointTable(joint_table);

    m_systems->postUpdate();
}

void GenerateBoxDialog::createFollowThicknessValueHandlers(PanelConfigurationManager& mgr) {
    using floatPtr = adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput>&;
    using boolPtr = adsk::core::Ptr<adsk::core::BoolValueCommandInput>&;

    auto default_thickness = m_configuration.ctx<DialogThicknessInput>().control;

    for (auto &&[entity, override, thickness]: mgr.overrideThicknessInputs()) {
        PLOG_DEBUG << "Creating follow thickness handler";

        auto callback = [](boolPtr override, floatPtr thickness, floatPtr default_thickness) {
            return [override, thickness](entt::registry &registry) {
                auto const &default_thickness = registry.ctx<DialogThicknessInput>().control;

                auto thickness_enabled = override->value();

                if (thickness_enabled) {
                    return;
                }

                thickness->value(default_thickness->value());
            };
        };

        addInputControl(default_thickness, callback(override.control, thickness.control, default_thickness));
    }
}

void GenerateBoxDialog::createOverrideEnableHandlers(PanelConfigurationManager &mgr) {
    using floatPtr = adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput>&;
    using boolPtr = adsk::core::Ptr<adsk::core::BoolValueCommandInput>&;

    for (auto &&[entity, enable, override]: mgr.enableOverrideInputs()) {

        auto callback = [](boolPtr override, boolPtr enable) {
            return [override, enable](entt::registry &registry) {
                override->isEnabled(enable->value());
            };
        };

        addInputControl(enable.control, callback(override.control, enable.control));
        addCollisionHandler(enable.control);
    }
}

void GenerateBoxDialog::createThicknessEnableHandlers(PanelConfigurationManager &mgr) {
    using floatPtr = adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput>&;
    using boolPtr = adsk::core::Ptr<adsk::core::BoolValueCommandInput>&;

    for (auto &&[entity, enable, override, thickness]: mgr.allThicknessInputs()) {

        auto callback = [](boolPtr override, floatPtr thickness) {
            return [override = override, thickness = thickness](entt::registry &registry) {
                thickness->isEnabled(override->value() && override->isEnabled());
            };
        };

        addInputControl(override.control, callback(override.control, thickness.control));
        addInputControl(enable.control, callback(override.control, thickness.control));
        addCollisionHandler(override.control);
        addCollisionHandler(thickness.control);
    }
}

void GenerateBoxDialog::addInputControl(DialogInputs reference, const adsk::core::Ptr<CommandInput>& input) {
    m_inputs[reference] = input->id();
}

void GenerateBoxDialog::addInputControl(const adsk::core::Ptr<CommandInput>& input, const std::function<void(entt::registry&)> &handler) {
    m_handlers[input->id()].emplace_back(handler);
}

void GenerateBoxDialog::addInputControl(const adsk::core::Ptr<CommandInput>& input, const std::vector<std::function<void(entt::registry&)>> &handlers) {
    for (auto const& handler: handlers) {
        m_handlers[input->id()].emplace_back(handler);
    }
}

void GenerateBoxDialog::addInputControl(DialogInputs reference, const adsk::core::Ptr<CommandInput>& input, const std::function<void(entt::registry&)> &handler) {
    addInputControl(reference, input);
    m_handlers[input->id()].emplace_back(handler);
    m_inputs[reference] = input->id();
}

void GenerateBoxDialog::addInputControl(
    const DialogInputs reference,
    const adsk::core::Ptr<CommandInput> &input,
    const std::vector<std::function<void(entt::registry& registry)>> &handlers
) {
    addInputControl(reference, input);
    for (auto const &handler: handlers) {
        m_handlers[input->id()].emplace_back(handler);
    }
}

void GenerateBoxDialog::addInputHandler(const DialogInputs reference, const std::function<void(entt::registry&)> &handler) {
    m_handlers[m_inputs[reference]].emplace_back(handler);
}

void GenerateBoxDialog::createModelSelectionDropDown(const Ptr<CommandInputs> &inputs) {
    auto creation_mode = inputs->addDropDownCommandInput("creationTypeCommandInput", "Type of Model", TextListDropDownStyle);
    m_configuration.set<DialogCreationMode>(creation_mode);

    auto const &creation_items = creation_mode->listItems();
    creation_items->add("Parametric", true);
    creation_items->add("Direct Model", false);
    creation_mode->maxVisibleItems(2);
}

void GenerateBoxDialog::createFingerModeSelectionDropDown(const Ptr<CommandInputs> &inputs) {
    auto finger_mode = inputs->addDropDownCommandInput("fingerTypeCommandInput", "Finger Size", TextListDropDownStyle);
    m_configuration.set<DialogFingerMode>(finger_mode);

    auto const &creation_items = finger_mode->listItems();
    creation_items->add("Automatic Width", true);
    creation_items->add("Constant Width", false);
    creation_items->add("None", false);
    finger_mode->maxVisibleItems(3);

    addInputControl(DialogInputs::FingerMode, finger_mode);
}

auto GenerateBoxDialog::createDimensionGroup(const adsk::core::Ptr<CommandInputs> &inputs) -> adsk::core::Ptr<GroupCommandInput> {
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

void GenerateBoxDialog::createDimensionParameters() {
    auto length       = m_configuration.ctx<DialogLengthInput>().control;
    auto width        = m_configuration.ctx<DialogWidthInput>().control;
    auto height       = m_configuration.ctx<DialogHeightInput>().control;
    auto thickness    = m_configuration.ctx<DialogThicknessInput>().control;
    auto finger_width = m_configuration.ctx<DialogFingerWidthInput>().control;
    auto kerf         = m_configuration.ctx<DialogKerfInput>().control;

    createFloatParameter("length", length);
    createFloatParameter("width", width);
    createFloatParameter("height", height);
    createFloatParameter("default_thickness", thickness);
    createFloatParameter("finger_width", finger_width);
    createFloatParameter("kerf", kerf);
}

void GenerateBoxDialog::createFloatParameter(std::string name, Ptr<FloatSpinnerCommandInput> control) {
    auto entity = m_configuration.create();
    m_configuration.emplace<FloatParameterInput>(entity, boost::algorithm::to_lower_copy(name), control);
}

auto GenerateBoxDialog::createDimensionInput(Ptr<CommandInputs> &children, const InputConfig &config) -> Ptr<FloatSpinnerCommandInput> {

    auto spinner = Ptr<FloatSpinnerCommandInput>{
        children->addFloatSpinnerCommandInput(
            config.id, config.name, config.unit_type, config.minimum,
            config.maximum, config.step, config.initial_value
        )
    };

    auto validator = [spinner](entt::registry& registry) {
        return spinner->value() >= spinner->minimumValue();
    };
    m_validators.emplace_back(validator);

    addInputControl(config.lookup, spinner);

    return spinner;
}

void GenerateBoxDialog::createPanelTable(
    const Ptr<CommandInputs> &inputs, PanelConfigurationManager& config_mgr
) {
    auto table             = initializePanelTable(inputs);
    auto is_metric         = m_configuration.ctx<DialogModelingUnits>().value;
    auto default_thickness = m_configuration.ctx<DialogThicknessInput>().control;

    for (auto &&[entity, panel, label_config, enable_config, override_config, thickness_config]: config_mgr.panels())
    {
        PLOG_DEBUG << "Adding " << panel.name << " option to input dialog.";
        auto row_num           = table->rowCount();

        auto label_control    = addPanelLabelControl(inputs, label_config);
        auto enable_control   = addPanelEnableControl(inputs, enable_config);
        auto override_control = addPanelOverrideControl(inputs, override_config);
        override_control->isEnabled(enable_control->value());

        auto thickness_control = inputs->addFloatSpinnerCommandInput(
            thickness_config.id, thickness_config.name, thickness_config.unit_type, thickness_config.minimum,
            thickness_config.maximum, thickness_config.step, thickness_config.initial_value
        );
        thickness_control->isEnabled(enable_control->value() && override_control->value());

        table->addCommandInput(label_control, row_num, 0, 0, 0);
        table->addCommandInput(enable_control, row_num, 2, 0, 0);
        table->addCommandInput(override_control, row_num, 5, 0, 0);
        table->addCommandInput(thickness_control, row_num, 7, 0, 0);

        auto parameter = thickness_config.name + "_thickness";
        auto controls = config_mgr.addControls(entity);
        controls.addLabel(label_control)
                .addEnable(enable_control)
                .addOverride(override_control)
                .addThickness(thickness_control)
                .addActiveThickness(default_thickness, parameter);
    }
}

auto GenerateBoxDialog::initializePanelTable(const adsk::core::Ptr<CommandInputs> &inputs) const -> adsk::core::Ptr<TableCommandInput> {
    auto table = inputs->addTableCommandInput(
        m_dimensions_table.id, m_dimensions_table.name, 0, m_dimensions_table.column_ratio
    );

    auto const num_rows = m_dimensions_table.dimensions.size() + 1;
    table->maximumVisibleRows((int) num_rows);
    table->minimumVisibleRows((int) num_rows);
    table->isEnabled(false);
    table->tablePresentationStyle(TablePresentationStyles::transparentBackgroundTablePresentationStyle);

    addTableTitles(table);
    return table;
}

void GenerateBoxDialog::addTableTitles(adsk::core::Ptr<TableCommandInput> &table) const {
    for (const auto &title: m_dimensions_table.titles) {
        auto title_input = table->commandInputs()->addTextBoxCommandInput(
            title.id, title.name, "<b>" + title.label + "</b>", 1, true
        );
        table->addCommandInput(title_input, 0, title.column, 0, title.span);
    }
}

auto GenerateBoxDialog::addPanelLabelControl(const Ptr<CommandInputs> &inputs, const PanelLabelConfig &row) -> Ptr<TextBoxCommandInput> {
    auto label_control = inputs->addTextBoxCommandInput(
        row.id, row.name, "<b>" + row.name + "</b>", 1, true
    );
    return label_control;
}

auto GenerateBoxDialog::addPanelEnableControl(const Ptr<CommandInputs> &inputs, const PanelEnableConfig &row) -> Ptr<BoolValueCommandInput> {
    auto enable_control = inputs->addBoolValueInput(
        row.id, row.name, true, "", row.default_value
    );
    return enable_control;
}

auto GenerateBoxDialog::addPanelOverrideControl(const Ptr<CommandInputs> &inputs, const PanelOverrideConfig &row) -> Ptr<BoolValueCommandInput> {
    auto override_control = inputs->addBoolValueInput(
        row.id, row.name, true, "", false
    );
    return override_control;
}

void GenerateBoxDialog::createDividerInputs(const Ptr<CommandInputs> &inputs) {
    createDividerOrientationsInput(inputs);
    createDividerJointDirectionInput(inputs);

    createLengthDividerInputs(inputs);
    createWidthDividerInputs(inputs);
    createHeightDividerInputs(inputs);
}

void GenerateBoxDialog::createHeightDividerInputs(const Ptr<CommandInputs> &inputs) {
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
        DialogInputs::HeightDividerFBJointInput, height_divider_fb_outside_joint, [this](entt::registry& registry) {
            updateDividers(registry);
        }
    );

    auto height_divider_lr_outside_joint = height_children->addDropDownCommandInput(
        "heightDividerOutsideLRJointInput", "Left/Right Joints", TextListDropDownStyle
    );
    m_configuration.set<DialogHeightDividerLeftRightJointInput>(height_divider_lr_outside_joint);
    addJointTypes(height_divider_lr_outside_joint);
    addInputControl(
        DialogInputs::HeightDividerLRJointInput, height_divider_lr_outside_joint, [this](entt::registry& registry) {
            updateDividers(registry);
        }
    );

    auto height = adsk::core::Ptr<IntegerSpinnerCommandInput>{
        height_children->addIntegerSpinnerCommandInput(
            "heightDividerCommandInput", "(#) Height Dividers", 0, 25, 1, 0
        )
    };
    m_configuration.set<DialogHeightDividerCountInput>(height);

    addInputControl(
        DialogInputs::HeightDividerCount, height, [this](entt::registry& registry) {
            updateDividers(registry);
        }
    );
}

void GenerateBoxDialog::updateHeightDividers(entt::registry& registry) {
    auto orientations = registry.ctx<DialogDividerOrientationsInput>().control->selectedItem()->index();
    if (orientations == 0) return;

    auto const divider_height    = registry.ctx<DialogHeightInput>().control->value();
    auto const divider_inverted = registry.ctx<DialogDividerJointInput>().control->selectedItem()->index() == 0 ;

    auto old_view = registry.view<HeightDivider>();
    registry.destroy(old_view.begin(), old_view.end());

    auto static_view = registry.view<HeightDividerJoint>();
    registry.destroy(static_view.begin(), static_view.end());

    PLOG_DEBUG << "Updating Height Divider information";

    auto dividers = Dividers<HeightDivider, DialogHeightDividerCountInput>(registry, m_app);
    dividers.setAxis(0, 0, 1);
    dividers.setMaxOffset(divider_height);
    dividers.setNamePrefix("Height");
    dividers.setOrientation(AxisFlag::Height);
    dividers.addOrientation<HeightOrientation>();
    dividers.setPriority(0);
//    dividers.addMaxLength("length"); // TODO: Convert to user definable length
//    dividers.addMaxWidth("width"); // TODO: Convert to user definable width
    dividers.addMaxHeight("height"); // TODO: Convert to user definable height & thickness
    dividers.create();

    auto is_inverted = (orientations == 1 && divider_inverted) || (orientations == 2 && !divider_inverted);
    auto inside_direction = static_cast<JointDirectionType>(is_inverted);
    m_systems->updateCollisions();
    m_systems->findJoints<HeightDivider, HeightDividerJoint>();
    m_systems->updateJointPatternInputs<HeightDividerJoint, DialogHeightDividerFrontBackJointInput>(AxisFlag::Width);
    m_systems->updateJointPatternInputs<HeightDividerJoint, DialogHeightDividerLeftRightJointInput>(AxisFlag::Length);
    m_systems->updateJointDirection<HeightDividerJoint>(Position::Outside, Position::Inside, JointDirectionType::Normal);
    m_systems->updateJointDirection<HeightDividerJoint>(Position::Inside, Position::Inside, inside_direction);
    m_systems->postUpdate();
}

void GenerateBoxDialog::createWidthDividerInputs(const Ptr<CommandInputs> &inputs) {
    auto width_group    = inputs->addGroupCommandInput("widthDividerGroupInput", "Width Dividers");
    auto width_children = width_group->children();
    m_configuration.set<DialogWidthDividerGroupInput>(width_group);

    auto width_divider_lr_outside_joint = width_children->addDropDownCommandInput(
        "widthDividerOutsideLRJointInput", "Left/Right Joints", TextListDropDownStyle
        );
    m_configuration.set<DialogWidthDividerLeftRightJointInput>(width_divider_lr_outside_joint);
    addJointTypes(width_divider_lr_outside_joint);
    addInputControl(
        DialogInputs::WidthDividerLRJointInput, width_divider_lr_outside_joint, [this](entt::registry& registry) {
            updateDividers(registry);
        }
    );

    auto width_divider_tb_outside_joint = width_children->addDropDownCommandInput(
        "widthDividerOutsideTBJointInput", "Top/Bottom Joints", TextListDropDownStyle
        );
    m_configuration.set<DialogWidthDividerTopBottomJointInput>(width_divider_tb_outside_joint);
    addJointTypes(width_divider_tb_outside_joint);
    addInputControl(
        DialogInputs::WidthDividerTBJointInput, width_divider_tb_outside_joint, [this](entt::registry& registry) {
            updateDividers(registry);
        }
    );

    auto width = adsk::core::Ptr<IntegerSpinnerCommandInput>{
        width_children->addIntegerSpinnerCommandInput(
            "widthDividerCommandInput", "(#) Width Dividers", 0, 25, 1, 0
        )
    };
    m_configuration.set<DialogWidthDividerCountInput>(width);

    addInputControl(
        DialogInputs::WidthDividerCount, width, [this](entt::registry& registry) {
            updateDividers(registry);
        }
    );
}

void GenerateBoxDialog::updateWidthDividers(entt::registry& registry) {
    auto orientations = registry.ctx<DialogDividerOrientationsInput>().control->selectedItem()->index();
    if (orientations == 1) return;

    auto const divider_width    = registry.ctx<DialogWidthInput>().control->value();
    auto const divider_inverted = registry.ctx<DialogDividerJointInput>().control->selectedItem()->index() == 1;

    auto old_view = registry.view<WidthDivider>();
    registry.destroy(old_view.begin(), old_view.end());

    auto static_view = registry.view<WidthDividerJoint>();
    registry.destroy(static_view.begin(), static_view.end());

    auto dividers = Dividers<WidthDivider, DialogWidthDividerCountInput>(registry, m_app);
    dividers.setAxis(0, 1, 0);
    dividers.setMaxOffset(divider_width);
    dividers.setNamePrefix("Width");
    dividers.setOrientation(AxisFlag::Width);
    dividers.addOrientation<WidthOrientation>();
    dividers.setPriority(1);
//    dividers.addMaxLength("length"); // TODO: Convert to user definable length
//    dividers.addMaxHeight("height"); // TODO: Convert to user definable height
    dividers.addMaxWidth("width"); // TODO: Convert to user definable width & thickness
    dividers.create();

    auto is_inverted = (orientations == 0 && !divider_inverted) || (orientations == 2 && divider_inverted);
    auto inside_direction = static_cast<JointDirectionType>(is_inverted);
    m_systems->updateCollisions();
    m_systems->findJoints<WidthDivider, WidthDividerJoint>();
    m_systems->updateJointPatternInputs<WidthDividerJoint, DialogWidthDividerLeftRightJointInput>(AxisFlag::Length);
    m_systems->updateJointPatternInputs<WidthDividerJoint, DialogWidthDividerTopBottomJointInput>(AxisFlag::Height);
    m_systems->updateJointDirection<WidthDividerJoint>(Position::Outside, Position::Inside, JointDirectionType::Normal);
    m_systems->updateJointDirection<WidthDividerJoint>(Position::Inside, Position::Inside, inside_direction);
    m_systems->postUpdate();    
}

void GenerateBoxDialog::createLengthDividerInputs(const Ptr<CommandInputs> &inputs) {
    auto length_group    = inputs->addGroupCommandInput("lengthDividerGroupInput", "Length Dividers");
    auto length_children = length_group->children();
    m_configuration.set<DialogLengthDividerGroupInput>(length_group);

    auto length_divider_fb_outside_joint = length_children->addDropDownCommandInput(
        "lengthDividerOutsideFBJointInput", "Front/Back Joints", TextListDropDownStyle
        );
    m_configuration.set<DialogLengthDividerFrontBackJointInput>(length_divider_fb_outside_joint);
    addJointTypes(length_divider_fb_outside_joint);
    addInputControl(
        DialogInputs::LengthDividerFBJointInput, length_divider_fb_outside_joint, [this](entt::registry& registry) {
            updateDividers(registry);
        }
    );

    auto length_divider_tb_outside_joint = length_children->addDropDownCommandInput(
        "lengthDividerOutsideTBJointInput", "Top/Bottom Joints", TextListDropDownStyle
        );
    m_configuration.set<DialogLengthDividerTopBottomJointInput>(length_divider_tb_outside_joint);
    addJointTypes(length_divider_tb_outside_joint);
    addInputControl(
        DialogInputs::LengthDividerTBJointInput, length_divider_tb_outside_joint, [this](entt::registry& registry) {
            updateDividers(registry);
        }
    );

    auto length = adsk::core::Ptr<IntegerSpinnerCommandInput>{
        length_children->addIntegerSpinnerCommandInput(
            "lengthDividerCommandInput", "(#) Length Dividers", 0, 25, 1, 0
        )
    };
    m_configuration.set<DialogLengthDividerCountInput>(length);

    addInputControl(
        DialogInputs::LengthDividerCount, length, [this](entt::registry& registry) {
            updateDividers(registry);
        }
    );
}

void GenerateBoxDialog::updateDividers(entt::registry& registry) {
    updateLengthDividers(registry);
    updateWidthDividers(registry);
    updateHeightDividers(registry);
}

void GenerateBoxDialog::updateLengthDividers(entt::registry& registry) {
    auto orientations = registry.ctx<DialogDividerOrientationsInput>().control->selectedItem()->index();
    if (orientations == 2) return;

    auto const divider_length   = registry.ctx<DialogLengthInput>().control->value();
    auto const divider_inverted = registry.ctx<DialogDividerJointInput>().control->selectedItem()->index() == 1;

    auto old_view = registry.view<LengthDivider>();
    registry.destroy(old_view.begin(), old_view.end());

    auto static_view = registry.view<LengthDividerJoint>();
    registry.destroy(static_view.begin(), static_view.end());

    PLOG_DEBUG << "Updating Length Divider information";

    auto dividers = Dividers<LengthDivider, DialogLengthDividerCountInput>(registry, m_app);
    dividers.setAxis(1, 0, 0);
    dividers.setMaxOffset(divider_length);
    dividers.setNamePrefix("Length");
    dividers.setOrientation(AxisFlag::Length);
    dividers.addOrientation<LengthOrientation>();
    dividers.setPriority(2);
//    dividers.addMaxWidth("width"); // TODO: Convert to user definable width
//    dividers.addMaxHeight("height"); // TODO: Convert to user definable height
    dividers.addMaxLength("length"); // TODO: Convert to user definable length & thickness
    dividers.create();
//    pocket_offset * divider_num + divider_thickness * (divider_num + 1);
//                    ((((length - thickness * ({0} + 2)) / ({0} + 1)) * {1}) + (thickness * ({1} + 1)))

    auto is_inverted = (orientations == 0 && divider_inverted) || (orientations == 1 && !divider_inverted);
    auto inside_direction = static_cast<JointDirectionType>(is_inverted);
    m_systems->updateCollisions();
    m_systems->findJoints<LengthDivider, LengthDividerJoint>();
    m_systems->updateJointPatternInputs<LengthDividerJoint, DialogLengthDividerFrontBackJointInput>(AxisFlag::Width);
    m_systems->updateJointPatternInputs<LengthDividerJoint, DialogLengthDividerTopBottomJointInput>(AxisFlag::Height);
    m_systems->updateJointDirection<LengthDividerJoint>(Position::Outside, Position::Inside, JointDirectionType::Normal);
    m_systems->updateJointDirection<LengthDividerJoint>(Position::Inside, Position::Inside, inside_direction);    
}

void GenerateBoxDialog::createDividerJointDirectionInput(const Ptr<CommandInputs> &inputs) {
    auto divider_joint = inputs->addDropDownCommandInput("dividerLapCommandInput", "Divider Joint", TextListDropDownStyle);
    m_configuration.set<DialogDividerJointInput>(divider_joint);
    auto const joint_items = divider_joint->listItems();
    joint_items->add("Normal", true);
    joint_items->add("Inverse", false);
    divider_joint->maxVisibleItems(2);

    addInputControl(
        DialogInputs::DividerLapInput, divider_joint, [this](entt::registry& registry) {
            update(registry.ctx<DialogLengthDividerCountInput>().control);
            update(registry.ctx<DialogWidthDividerCountInput>().control);
            update(registry.ctx<DialogHeightDividerCountInput>().control);
        }
    );
}

void GenerateBoxDialog::createDividerOrientationsInput(const Ptr<CommandInputs> &inputs) {
    auto divider_orientations = inputs->addDropDownCommandInput("dividerOrientationCommandInput", "Divider Orientations", TextListDropDownStyle);
    m_configuration.set<DialogDividerOrientationsInput>(divider_orientations);
    auto const type_items = divider_orientations->listItems();
    type_items->add("Length & Width", true);
    type_items->add("Length & Height", false);
    type_items->add("Width & Height", false);
    divider_orientations->maxVisibleItems(3);

    addInputControl(
        DialogInputs::DividerOrientations, divider_orientations, [](entt::registry& registry) {
            auto orientations = registry.ctx<DialogDividerOrientationsInput>().control->selectedItem()->index();

            auto length_group_input = registry.ctx<DialogLengthDividerGroupInput>().control;
            auto length_count_input = registry.ctx<DialogLengthDividerCountInput>().control;
            auto width_group_input = registry.ctx<DialogWidthDividerGroupInput>().control;
            auto width_count_input = registry.ctx<DialogWidthDividerCountInput>().control;
            auto height_group_input = registry.ctx<DialogHeightDividerGroupInput>().control;
            auto height_count_input = registry.ctx<DialogHeightDividerCountInput>().control;

            auto selector = std::map<int, std::function<void(entt::registry& registry)>>{
                {0, [&](entt::registry& registry){
                    height_count_input->value(0);
                    auto old_view = registry.view<HeightDivider>();
                    registry.destroy(old_view.begin(), old_view.end());
            }},
                {1, [&](entt::registry& registry){
                    width_count_input->value(0);
                    auto old_view = registry.view<WidthDivider>();
                    registry.destroy(old_view.begin(), old_view.end());
            }},
                {2, [&](entt::registry& registry){
                    length_count_input->value(0);
                    auto old_view = registry.view<LengthDivider>();
                    registry.destroy(old_view.begin(), old_view.end());
                }}
            };

            auto length_is_valid = orientations < 2;
            auto width_is_valid = orientations == 0 || orientations == 2;
            auto height_is_valid = orientations > 0;

            length_group_input->isVisible(length_is_valid);
            width_group_input->isVisible(width_is_valid);
            height_group_input->isVisible(height_is_valid);

            selector[orientations](registry);
        }
    );
}

auto GenerateBoxDialog::createPanelOffsetTable(const adsk::core::Ptr<CommandInputs> &inputs) -> adsk::core::Ptr<TableCommandInput> {
    auto standard_group = inputs->addGroupCommandInput("standardPanelsOffsetGroupCommandInput", "Standard");
    auto group_children = standard_group->children();
    auto table = group_children->addTableCommandInput(
        "panelOffsetTableCommandInput", "Offsets", 0, "1:1:1:1"
    );
    table->maximumVisibleRows(6);
    table->minimumVisibleRows(6);
    table->tablePresentationStyle(TablePresentationStyles::itemBorderTablePresentationStyle);

    auto const panel_label    = table->commandInputs()->addTextBoxCommandInput("panelOffsetLabelInput", "Panel", "<b>Panel</b>", 1, true);
    auto const inset_label   = table->commandInputs()->addTextBoxCommandInput("panelInsetLabelInput", "Inset", "<b>Inset</b>", 1, true);
    auto const side_label     = table->commandInputs()->addTextBoxCommandInput("panelSideOffsetLabelInput", "Side Offset", "<b>Side Offset</b>", 1, true);
    auto const start_label  = table->commandInputs()->addTextBoxCommandInput("panelStartOffsetLabelInput", "Start Offset", "<b>Start Offset</b>", 1, true);

    table->addCommandInput(panel_label, 0, 0);
    table->addCommandInput(inset_label, 0, 1);
    table->addCommandInput(side_label, 0, 2);
    table->addCommandInput(start_label, 0, 3);
    return table;
}

auto GenerateBoxDialog::createStandardJointTable(const adsk::core::Ptr<CommandInputs> &inputs) -> adsk::core::Ptr<TableCommandInput> {
    auto joint_group = inputs->addGroupCommandInput("standardJointsGroupCommandInput", "Standard");
    auto group_children = joint_group->children();
    m_configuration.set<DialogStandardJointGroupInput>(joint_group);

    auto table = group_children->addTableCommandInput(
        "jointTableCommandInput", "Joints", 0, "1:1:1:1"
    );
    table->maximumVisibleRows(17);
    table->minimumVisibleRows(17);
    table->tablePresentationStyle(TablePresentationStyles::itemBorderTablePresentationStyle);

    auto const first_label    = table->commandInputs()->addTextBoxCommandInput("firstJointLabelInput", "First", "<b>First</b>", 1, true);
    auto const second_label   = table->commandInputs()->addTextBoxCommandInput("secondJointLabelInput", "Second", "<b>Second</b>", 1, true);
    auto const pattern_label  = table->commandInputs()->addTextBoxCommandInput("jointPatternLabelInput", "Pattern", "<b>Pattern</b>", 1, true);
    auto const type_label     = table->commandInputs()->addTextBoxCommandInput("jointPatternLabelInput", "Joint Type", "<b>Joint Type</b>", 1, true);

    table->addCommandInput(first_label, 0, 0);
    table->addCommandInput(second_label, 0, 1);
    table->addCommandInput(pattern_label, 0, 2);
    table->addCommandInput(type_label, 0, 3);
    return table;
}

void GenerateBoxDialog::createPreviewTable(const adsk::core::Ptr<CommandInputs> &inputs) {
    auto table = inputs->addTableCommandInput(
        "previewTableCommandInput", "Preview", 0, "1:5:1:5"
    );

    table->maximumVisibleRows((int) 1);
    table->minimumVisibleRows((int) 1);
    table->isEnabled(false);
    table->tablePresentationStyle(TablePresentationStyles::transparentBackgroundTablePresentationStyle);

    auto const fast_preview = table->commandInputs()->addBoolValueInput("fastPreviewCommandInput", "Preview", true, "", true);
    auto const fast_label   = table->commandInputs()->addTextBoxCommandInput("fastPreviewLabelInput", "Preview", "Preview", 1, true);

    m_configuration.set<DialogFastPreviewMode>(fast_preview);
    m_configuration.set<DialogFastPreviewLabel>(fast_label);

    table->addCommandInput(fast_preview, 0, 0);
    table->addCommandInput(fast_label, 0, 1);

    addInputControl(DialogInputs::FastPreviewLabel, fast_label);
}

void GenerateBoxDialog::addMinimumAxisDimensionChecks() {
    auto validator = [](entt::registry& registry) {
        auto length_ok = validateDimension<DialogLengthInput, LengthOrientation>(registry, AxisFlag::Length);
        auto width_ok  = validateDimension<DialogWidthInput, WidthOrientation>(registry, AxisFlag::Width);
        auto height_ok = validateDimension<DialogHeightInput, HeightOrientation>(registry, AxisFlag::Height);

        return length_ok && width_ok && height_ok;
    };
    m_validators.emplace_back(validator);
}

void GenerateBoxDialog::addMaximumKerfCheck() {
    auto validator = [](entt::registry& registry) {
        auto kerf = registry.ctx<DialogKerfInput>().control->value();

        bool kerf_ok = true;

        auto view = registry.view<const PanelThicknessInput>().proxy();
        for (auto &&[entity, thickness]: view) {
            kerf_ok = kerf_ok && (thickness.control->value() > kerf);
        }

        if (kerf_ok) { return true; }

        registry.set<DialogErrorMessage>(
            "<span style=\" font-weight:600; color:#ff0000;\">Kerf</span> is larger than the smallest panel thickness."
        );

        return false;
    };
    m_validators.emplace_back(validator);
}

void GenerateBoxDialog::addMinimumFingerWidthCheck() {
    auto validator = [](entt::registry& registry) {
        auto finger_width = registry.ctx<DialogFingerWidthInput>().control->value();

        bool finger_ok = true;

        auto view = registry.view<const PanelThicknessInput>().proxy();
        for (auto &&[entity, thickness]: view) {
            finger_ok = finger_ok && (thickness.control->value() < finger_width);
        }

        if (finger_ok) { return true; }

        registry.set<DialogErrorMessage>(
            "<span style=\" font-weight:600; color:#ff0000;\">Finger width</span> should be larger than the largest panel thickness."
        );

        return false;
    };
    m_validators.emplace_back(validator);
}

void GenerateBoxDialog::addMinimumPanelCountCheck() {
    auto validator = [](entt::registry& registry) {
        int thickness_count = 0;

        auto view = registry.view<const PanelEnableInput>().proxy();
        for (auto &&[entity, enable]: view) {
            thickness_count += enable.control->value();
        }

        if (thickness_count < 2) {
            registry.set<DialogErrorMessage>(
                "<span style=\" font-weight:600; color:#ff0000;\">A minimum of two panels must be selected."
            );
            return false;
        }

        return true;
    };
    m_validators.emplace_back(validator);
}

void GenerateBoxDialog::populateJointTable(Ptr<TableCommandInput> &table) {

    auto inputs  = table->commandInputs();
    auto row_num = 1;

    auto view = m_configuration.view<entities::JointPanels, entities::DialogJointPattern, entities::DialogPanelCollisionData>().proxy();
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

void GenerateBoxDialog::addJointTypes(Ptr<DropDownCommandInput> &dropdown) {
    auto const &items = dropdown->listItems();
    items->add("Box Joint", true);
    items->add("Lap Joint", false);
    items->add("Tenon", false);
    items->add("Double Tenon", false);
    items->add("Triple Tenon", false);
    items->add("Quad Tenon", false);
    items->add("Trim", false);
    items->add("None", false);
    dropdown->maxVisibleItems(7);
}

void GenerateBoxDialog::addCollisionHandler(DialogInputs reference) {
    auto handler = [this](entt::registry& registry) {
        m_systems->updateCollisions();
        m_systems->postUpdate();
    };

    addInputHandler(reference, handler);
}

void GenerateBoxDialog::addCollisionHandler(Ptr<BoolValueCommandInput>& reference) {
    auto handler = [this](entt::registry& registry) {
        m_systems->updateCollisions();
        m_systems->postUpdate();
    };

    addInputControl(reference, handler);
}

void GenerateBoxDialog::addCollisionHandler(Ptr<FloatSpinnerCommandInput>& reference) {
    auto handler = [this](entt::registry& registry) {
        m_systems->updateCollisions();
        m_systems->postUpdate();
    };

    addInputControl(reference, handler);
}


bool GenerateBoxDialog::validate(const adsk::core::Ptr<CommandInputs> &inputs) {
    auto m_error_message = m_configuration.ctx<DialogErrorMessage>().value;
    m_error->formattedText(m_error_message);

    auto results = all_of(m_validators.begin(), m_validators.end(), [this](const std::function<bool(entt::registry&)> &v) { return v(m_configuration); });

    m_error->isVisible(!results);

    return results;
}

bool GenerateBoxDialog::update(const adsk::core::Ptr<CommandInput> &cmd_input) {
    auto exists = std::find(m_ignore_updates.begin(), m_ignore_updates.end(), cmd_input->id()) != m_ignore_updates.end();
    if (exists) { return false; }

    auto handlers = m_handlers[cmd_input->id()];

    for (auto &handler: handlers) {
        handler(m_configuration);
    }

    m_systems->postUpdate();

    return true;
}

void GenerateBoxDialog::initializePanels() {
    m_panel_registry.clear();
    m_systems->initializePanels(m_panel_registry);
}

