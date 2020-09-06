//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/JointProfile.hpp"
#include "entities/JointPattern.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void updateJointProfilesFromJointPatterns(entt::registry &registry) {
    auto profile_pattern_view = registry.view<JointProfile, const JointPattern>().proxy();
    for (auto &&[entity, profile, pattern]: profile_pattern_view) {
        PLOG_DEBUG << "Updating JointProfile pattern";
        profile.joint_type = pattern.value;
    }
}

