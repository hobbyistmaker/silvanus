//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/JointPosition.hpp"
#include "entities/JointProfile.hpp"
#include "entities/PanelPosition.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void updateJointProfilesFromPanelAndJointPositions(entt::registry &registry) {
    auto joint_profile_position_view = registry.view<JointProfile, const PanelPosition, const JointPosition>().proxy();
    for (auto &&[entity, profile, panel, joint]: joint_profile_position_view) {
        PLOG_DEBUG << "Updating joint profile with panel and joint position";
        profile.panel_position = panel.value;
        profile.joint_position = joint.value;
    }
}