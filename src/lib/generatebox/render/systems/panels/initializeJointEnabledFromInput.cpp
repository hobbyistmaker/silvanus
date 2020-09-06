//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/JointEnabled.hpp"
#include "entities/JointProfile.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void initializeJointEnabledFromInput(entt::registry &registry) {
    auto joint_enabled_view = registry.view<const JointEnabledInput>().proxy();
    for (auto &&[entity, enabled]: joint_enabled_view) {
        PLOG_DEBUG << "Updating JointEnabled value";
        registry.emplace<JointEnabled>(entity, enabled.control->value());
    }
}