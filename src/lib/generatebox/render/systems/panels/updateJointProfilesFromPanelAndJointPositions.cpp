//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/JointPosition.hpp"
#include "entities/JointProfile.hpp"
#include "entities/PanelPosition.hpp"
#include "entities/ProgressDialogControl.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void updateJointProfilesFromPanelAndJointPositions(entt::registry &registry) {
    auto view = registry.view<JointProfile, const PanelPosition, const JointPosition>();

    auto progress = registry.try_ctx<ProgressDialogControl>();
    auto progress_value = 1;
    auto max_progress = view.size();
    if (progress) {
        progress->control->reset();
        progress->control->message("Configuring panel and joint positions...");
        progress->control->maximumValue(view.size());
    }

    for (auto &&[entity, profile, panel, joint]: view.proxy()) {
        PLOG_DEBUG << "Updating joint profile with panel and joint position";
        profile.panel_position = panel.value;
        profile.joint_position = joint.value;

        if (progress) progress->control->progressValue(progress_value);
        progress_value += 1;
    }

    if (progress) progress->control->progressValue(max_progress);
}