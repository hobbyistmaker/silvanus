//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/22/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//

#include "generateboxcommand.hpp"

#include "entities/Dimensions.hpp"
#include "entities/EndReferencePoint.hpp"
#include "entities/FingerPattern.hpp"
#include "entities/FingerWidth.hpp"
#include "entities/FingerWidthInput.hpp"
#include "entities/InsidePanel.hpp"
#include "entities/JointPatternTags.hpp"
#include "entities/JointThickness.hpp"
#include "entities/MaxOffset.hpp"
#include "entities/MaxOffsetInput.hpp"
#include "entities/OutsidePanel.hpp"
#include "entities/PanelPosition.hpp"
#include "entities/PanelProfile.hpp"
#include "entities/Position.hpp"
#include "entities/StartReferencePoint.hpp"
#include "entities/ToggleableThicknessInput.hpp"

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include "plog/Log.h"

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;
using namespace silvanus;

GenerateBoxCommand::GenerateBoxCommand(
    const adsk::core::Ptr<Application>& app
) : common::Fusion360Command(app),
    m_ui(m_app->userInterface()) {

    m_registry.on_construct<AutomaticFingerPatternType>().connect<
        &entt::registry::remove_if_exists<ConstantFingerPatternType, ConstantAdaptiveFingerPatternType, NoFingerPatternType>
    >();
    m_registry.on_construct<ConstantFingerPatternType>().connect<
        &entt::registry::remove_if_exists<AutomaticFingerPatternType, ConstantAdaptiveFingerPatternType, NoFingerPatternType>
    >();
    m_registry.on_construct<ConstantAdaptiveFingerPatternType>().connect<
        &entt::registry::remove_if_exists<AutomaticFingerPatternType, ConstantFingerPatternType, NoFingerPatternType>
    >();
    m_registry.on_construct<NoFingerPatternType>().connect<
        &entt::registry::remove_if_exists<ConstantFingerPatternType, ConstantAdaptiveFingerPatternType, AutomaticFingerPatternType>
    >();

    m_registry.on_construct<BoxJointPattern>().connect<
        &entt::registry::remove_if_exists<LapJointPattern, TrimJointPattern, NoJointPattern, TenonJointPattern, DoubleTenonJointPattern, TripleTenonJointPattern, QuadTenonJointPattern>
    >();
    m_registry.on_construct<LapJointPattern>().connect<
        &entt::registry::remove_if_exists<BoxJointPattern, TrimJointPattern, NoJointPattern, TenonJointPattern, DoubleTenonJointPattern, TripleTenonJointPattern, QuadTenonJointPattern>
    >();
    m_registry.on_construct<TrimJointPattern>().connect<
        &entt::registry::remove_if_exists<BoxJointPattern, LapJointPattern, NoJointPattern, TenonJointPattern, DoubleTenonJointPattern, TripleTenonJointPattern, QuadTenonJointPattern>
    >();
    m_registry.on_construct<TenonJointPattern>().connect<
        &entt::registry::remove_if_exists<BoxJointPattern, LapJointPattern, NoJointPattern, TrimJointPattern, DoubleTenonJointPattern, TripleTenonJointPattern, QuadTenonJointPattern>
    >();
    m_registry.on_construct<DoubleTenonJointPattern>().connect<
        &entt::registry::remove_if_exists<BoxJointPattern, LapJointPattern, NoJointPattern, TrimJointPattern, TenonJointPattern, TripleTenonJointPattern, QuadTenonJointPattern>
    >();
    m_registry.on_construct<QuadTenonJointPattern>().connect<
        &entt::registry::remove_if_exists<BoxJointPattern, LapJointPattern, NoJointPattern, TrimJointPattern, TenonJointPattern, DoubleTenonJointPattern, TripleTenonJointPattern>
    >();
    m_registry.on_construct<NoJointPattern>().connect<
        &entt::registry::remove_if_exists<BoxJointPattern, LapJointPattern, TrimJointPattern, TenonJointPattern, DoubleTenonJointPattern, TripleTenonJointPattern, QuadTenonJointPattern>
    >();
}

void GenerateBoxCommand::onCreate(const adsk::core::Ptr<CommandCreatedEventArgs>& args)
{
    auto command = args->command();

    auto product = adsk::core::Ptr<Product>{m_app->activeProduct()};
    auto design = adsk::core::Ptr<Design>{product};

    command->setDialogMinimumSize(620, 620);
    command->setDialogInitialSize(620, 620);
    command->isRepeatable(false);
    command->okButtonText("Create");

    auto preferences = adsk::core::Ptr<Preferences>{m_app->preferences()};
    auto units_manager = adsk::core::Ptr<FusionUnitsManager>{design->fusionUnitsManager()};

    auto inputs = command->commandInputs();
    auto use_metric = (units_manager->distanceDisplayUnits() < InchDistanceUnits);
    auto root_component = design->rootComponent();
    auto orientation = preferences->generalPreferences()->defaultModelingOrientation();

    command_dialog.create(m_app, inputs, root_component, orientation, use_metric);
}

bool GenerateBoxCommand::onChange(const adsk::core::Ptr<InputChangedEventArgs>& args) {
    auto cmd_input = adsk::core::Ptr<CommandInput>{args->input()};
    return command_dialog.update(cmd_input);
}

void GenerateBoxCommand::onDestroy(const adsk::core::Ptr<CommandEventArgs>& args) {
    command_dialog.clear();
    m_registry.clear();
}

void GenerateBoxCommand::onExecute(const adsk::core::Ptr<CommandEventArgs>& args) {
    auto preferences = adsk::core::Ptr<Preferences>{m_app->preferences()};
    auto product = adsk::core::Ptr<Product>{m_app->activeProduct()};
    auto design = adsk::core::Ptr<Design>{product};
    auto root_component = design->rootComponent();
    auto orientation = preferences->generalPreferences()->defaultModelingOrientation();

    command_dialog.initializePanels();

    m_core.execute(orientation, root_component, command_dialog.is_parametric());
}

void GenerateBoxCommand::onPreview(const adsk::core::Ptr<CommandEventArgs>& args) {
    if (!command_dialog.full_preview() && !command_dialog.fast_preview()) return;

    command_dialog.initializePanels();

    auto preferences = adsk::core::Ptr<Preferences>{m_app->preferences()};
    auto product = adsk::core::Ptr<Product>{m_app->activeProduct()};
    auto design = adsk::core::Ptr<Design>{product};
    auto root_component = design->rootComponent();
    auto orientation = preferences->generalPreferences()->defaultModelingOrientation();

    if (command_dialog.full_preview()) {
        m_core.full_preview(orientation, root_component);
    } else {
        m_core.fast_preview(orientation, root_component);
    }
}

bool GenerateBoxCommand::onValidate(const adsk::core::Ptr<ValidateInputsEventArgs>& args) {
    return command_dialog.validate(args);
}

void GenerateBoxCommand::postValidateValid(const adsk::core::Ptr<CommandEventArgs>& args) {
    // TODO GenerateBoxCommand::postValidateValid
}
