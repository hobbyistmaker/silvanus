//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/JointOrientation.hpp"
#include "entities/JointProfile.hpp"
#include "entities/OrientationGroup.hpp"
#include "entities/Panel.hpp"
#include "entities/ProgressDialogControl.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void updateJointProfilesFromPanelAndJointOrientations(entt::registry &registry) {
    auto view = registry.view<JointProfile, const Panel, const JointOrientation>();

    auto progress = registry.try_ctx<ProgressDialogControl>();
    auto progress_value = 1;
    auto max_progress = view.size();
    if (progress) {
        progress->control->reset();
        progress->control->message("Configuration panel and joint orientations...");
        progress->control->maximumValue(view.size());
    }

    for (auto &&[entity, profile, panel, joint]: view.proxy()) {
        PLOG_DEBUG << "Add orientation group for " << panel.name;
        profile.panel_orientation = panel.orientation;
        profile.joint_orientation = joint.axis;
        registry.emplace<OrientationGroup>(entity, panel.orientation, joint.axis);

        if (progress) progress->control->progressValue(progress_value);
        progress_value += 1;
    }

    if (progress) progress->control->progressValue(max_progress);
}