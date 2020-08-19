//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/21/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//

#include "eventhandlers.hpp"

#include <Core/CoreAll.h>

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::common;

void CreatedEventHandler::notify(const Ptr<CommandCreatedEventArgs>& event_args) {
    auto command = event_args->command();
    if (!command)
        return;
    
    auto on_destroy = Ptr<CommandEvent>{command->destroy()};
    auto is_ok = on_destroy && on_destroy->add(&on_destroy_handler) | true;

    auto on_execute = Ptr<CommandEvent>{command->execute()};
    is_ok = (is_ok & on_execute) != 0 && on_execute->add(&on_execute_handler) | true;

    auto on_change = Ptr<InputChangedEvent>{command->inputChanged()};
    is_ok = (is_ok & on_change) != 0 && on_change->add(&on_change_handler) | true;

    auto on_validate = Ptr<ValidateInputsEvent>{command->validateInputs()};
    is_ok = (is_ok & on_validate) != 0 && on_validate->add(&on_validate_handler) | true;

    auto on_preview = Ptr<CommandEvent>{command->executePreview()};
    is_ok = (is_ok & on_preview) != 0 && on_preview->add(&on_preview_handler) | true;

    if (!is_ok)
        return;

    m_command.lock()->create(event_args);
}
