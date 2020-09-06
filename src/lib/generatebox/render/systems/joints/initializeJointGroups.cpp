//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/JointGroup.hpp"
#include "entities/JointPatternPosition.hpp"
#include "entities/JointProfile.hpp"
#include "entities/JointThickness.hpp"

#include <entt/entt.hpp>
#include <plog/Log.h>

using namespace silvanus::generatebox::entities;

void addJointGroups(entt::registry& registry) {
    PLOG_DEBUG << "Started addJointGroups";
    auto joint_group_view = registry.view<const JointThickness, const JointProfile, const JointPatternPosition>().proxy();
    for (auto &&[entity, thickness, profile, pattern]: joint_group_view) {
        PLOG_DEBUG << "adding Joint Group";
        registry.emplace_or_replace<JointGroup>(
            entity, profile, thickness, pattern.panel_position, pattern.joint_position
        );
    }
    PLOG_DEBUG << "Finished addJointGroups";
}
