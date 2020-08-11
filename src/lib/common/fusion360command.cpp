//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/21/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//
#include <Core/CoreAll.h>
#include "fusion360command.hpp"

using namespace adsk::core;
using namespace adsk::fusion;
using namespace silvanus::common;

Fusion360Command::Fusion360Command(
    const Ptr<Application> app
) : m_app(app) {
    
    valid = true;
}

void Fusion360Command::change(const Ptr<InputChangedEventArgs>& args) {
    dirty = onChange(args);
}

void Fusion360Command::create(const Ptr<CommandCreatedEventArgs>& args) {
    onCreate(args);
}

void Fusion360Command::destroy(const Ptr<CommandEventArgs>& args) {
    onDestroy(args);
}

void Fusion360Command::deactivate(const Ptr<CommandEventArgs>& args) {
    onDeactivate(args);
}

void Fusion360Command::execute(const Ptr<CommandEventArgs>& args) {
    onExecute(args);
}

bool Fusion360Command::preview(const Ptr<CommandEventArgs>& args) {
    postValidateValid(args);
    dirty = false;
    onPreview(args);
    return false;
}

bool Fusion360Command::validate(const Ptr<ValidateInputsEventArgs>& args) {
    if (dirty)
        valid = onValidate(args);
    return valid;
}
