//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/DialogInputs.hpp"
#include "entities/FingerPattern.hpp"

#include <plog/Log.h>

#include "DialogSystemManager.hpp"
#include "FingerPattern.hpp"

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::dialog;

void updateFingerPatternTypeImpl(entt::registry& registry) {
    auto finger_pattern_view = registry.view<FingerPattern, const DialogFingerMode>().proxy();
    for (auto &&[entity, pattern, finger_mode]: finger_pattern_view) {
        pattern.value = static_cast<FingerPatternType>(finger_mode.control->selectedItem()->index());
        PLOG_DEBUG << "Updating FingerPattern Type for entity " << (int)entity << " == " << (int)pattern.value;
    }
}
