//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/21/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//

#include <Core/CoreAll.h>

#include "commandbutton.hpp"

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::common;

Ptr<CommandDefinition> CommandButton::addButtonDefinition() {
    auto ui = m_app->userInterface();
    auto cmd_defs = ui->commandDefinitions();
    
    return cmd_defs->addButtonDefinition(
        m_command->getId(), m_command->getDialogName(), m_command->getDescription()
    );
}

void CommandButton::addControlToPanel() {
    Ptr<UserInterface> ui = m_app->userInterface();

    auto control = commandControl();
    if (control) {
        return;
    }

    scriptsPanel()->controls()->addCommand(findOrCreateCommandDefinition());
}

Ptr<CommandControl> CommandButton::commandControl() {
    Ptr<UserInterface> ui = m_app->userInterface();

    auto panel = scriptsPanel();
    auto cmd_def = commandDefinition();
    
    if (!cmd_def) {
        return nullptr;
    }
    
    auto controls = panel->controls();
    auto control = controls->itemById(cmd_def->id());
    
    return control;
}

Ptr<CommandDefinition> CommandButton::commandDefinition() {
    Ptr<UserInterface> ui = m_app->userInterface();
    Ptr<CommandDefinitions> cmd_defs = ui->commandDefinitions();
    Ptr<CommandDefinition> cmd_def = cmd_defs->itemById(m_command->getId());
    
    return cmd_def;
}

void CommandButton::delCommandButton() {
    auto scripts_panel = scriptsPanel();
    auto cmd_def = commandDefinition();
    auto control = commandControl();
    
    if (cmd_def) {
        cmd_def->deleteMe();
    }
    
    if (control) {
        control->deleteMe();
    }
}

Ptr<CommandDefinition> CommandButton::findOrCreateCommandDefinition() {
    auto cmd_def = commandDefinition();
    return cmd_def ? cmd_def : addButtonDefinition();
}

void CommandButton::registerHandler() {
    Ptr<UserInterface> ui = m_app->userInterface();
   
    auto cmd_def = findOrCreateCommandDefinition();
    bool is_ok = cmd_def->commandCreated()->add(m_handler.get());
    
    if (!cmd_def || !is_ok) {
        ui->messageBox("Unable to register handler for command.");
    }
}

Ptr<ToolbarPanel> CommandButton::scriptsPanel() {
    Ptr<UserInterface> ui = m_app->userInterface();
    Ptr<ToolbarPanelList> panels = ui->allToolbarPanels();
    auto item = panels->itemById(m_command->getPanelLocation());

    return panels->itemById(m_command->getPanelLocation());
}
