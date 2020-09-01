//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/DialogInputs.hpp"
#include "entities/JointDirection.hpp"

#include "DialogSystemManager.hpp"

#include <plog/Log.h>

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::dialog;

void updateJointDirectionImpl(entt::registry& registry) {
    auto direction_view = registry.view<JointDirections, DialogJointDirectionInputs>().proxy();
    for (auto &&[entity, directions, input]: direction_view) {
        auto result = (bool)input.first.control->selectedItem()->index();
        directions.first = static_cast<JointDirectionType>(result);
        directions.second = static_cast<JointDirectionType>(!result);
        PLOG_DEBUG << "Updating Joint Directions for entity " << (int)entity << ": first(" << (int)directions.first << "), second(" << (int)directions.second << ")";
    }
}
