//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/DialogInputs.hpp"
#include "entities/JointPattern.hpp"

#include "DialogSystemManager.hpp"

#include <plog/Log.h>

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::dialog;

void updateJointPatternImpl(entt::registry& registry)
{
    auto joint_pattern_view = registry.view<JointPattern, DialogJointPatternInput>().proxy();
    for (auto &&[entity, pattern, input]: joint_pattern_view) {
        pattern.value = static_cast<JointPatternType>((int)input.control->selectedItem()->index());
        PLOG_DEBUG << "Updating JointPattern for entity " << (int)entity << " == " << (int)pattern.value;
    }
}
