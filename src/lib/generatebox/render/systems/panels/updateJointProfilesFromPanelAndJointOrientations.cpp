//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/JointOrientation.hpp"
#include "entities/JointProfile.hpp"
#include "entities/OrientationGroup.hpp"
#include "entities/Panel.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void updateJointProfilesFromPanelAndJointOrientations(entt::registry &registry) {
    auto profile_orientation_view = registry.view<JointProfile, const Panel, const JointOrientation>().proxy();
    for (auto &&[entity, profile, panel, joint]: profile_orientation_view) {
        PLOG_DEBUG << "Add orientation group for " << panel.name;
        profile.panel_orientation = panel.orientation;
        profile.joint_orientation = joint.axis;
        registry.emplace<OrientationGroup>(entity, panel.orientation, joint.axis);
    }
}