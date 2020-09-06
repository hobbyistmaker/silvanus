//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/JointDirection.hpp"
#include "entities/JointProfile.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void updateJointProfilesFromJointDirections(entt::registry &registry) {
    auto joint_profile_direction_view = registry.view<JointProfile, const JointDirection>().proxy();
    for (auto &&[entity, profile, direction]: joint_profile_direction_view) {
        PLOG_DEBUG << "Setting Joint Profile direction for " << (int)entity << " to " << (int)direction.value;
        profile.joint_direction = direction.value;
    }
}