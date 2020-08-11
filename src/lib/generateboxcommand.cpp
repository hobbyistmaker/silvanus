//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/22/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//

#include "generateboxcommand.hpp"

#include "entities/FingerPatternType.hpp"

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

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
}

void GenerateBoxCommand::onCreate(const adsk::core::Ptr<CommandCreatedEventArgs>& args)
{
    auto command = args->command();

    auto product = adsk::core::Ptr<Product>{m_app->activeProduct()};
    auto design = adsk::core::Ptr<Design>{product};

    command->setDialogMinimumSize(329, 425);
    command->setDialogInitialSize(329, 425);
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

    m_core.execute(orientation, root_component, command_dialog.is_parametric());
}

void GenerateBoxCommand::onPreview(const adsk::core::Ptr<CommandEventArgs>& args) {
    if (!command_dialog.full_preview() && !command_dialog.fast_preview()) return;


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
