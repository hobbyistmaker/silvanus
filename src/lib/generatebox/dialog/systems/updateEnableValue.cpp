//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "DialogSystemManager.hpp"

#include "entities/DialogInputs.hpp"

#include <plog/Log.h>

#include "DialogSystemManager.hpp"

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::dialog;

void updateEnableValueImpl(entt::registry& registry) {
    auto view = registry.view<DialogPanelEnableValue, const DialogPanelEnable>().proxy();
    for (auto &&[entity, enable, input]: view) {
        enable.value = input.control->value();
        PLOG_DEBUG << "Updating panel enable value for entity " << (int)entity << " == " << (int)enable.value;
    }
}
