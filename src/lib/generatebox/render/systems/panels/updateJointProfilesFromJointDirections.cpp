//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/JointDirection.hpp"
#include "entities/JointProfile.hpp"
#include "entities/ProgressDialogControl.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void updateJointProfilesFromJointDirections(entt::registry &registry) {
    auto view = registry.view<JointProfile, const JointDirection>();
    auto progress = registry.try_ctx<ProgressDialogControl>();
    auto progress_value = 1;
    auto max_progress = view.size();
    if (progress) {
        progress->control->reset();
        progress->control->message("Configuring joint directions...");
        progress->control->maximumValue(view.size());
    }

    for (auto &&[entity, profile, direction]: view.proxy()) {
        PLOG_DEBUG << "Setting Joint Profile direction for " << (int)entity << " to " << (int)direction.value;
        profile.joint_direction = direction.value;

        if (progress) progress->control->progressValue(progress_value);
        progress_value += 1;
    }
    if (progress) progress->control->progressValue(max_progress);
}