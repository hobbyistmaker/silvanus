//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/JointEnabled.hpp"
#include "entities/JointProfile.hpp"
#include "entities/ProgressDialogControl.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void initializeJointEnabledFromInput(entt::registry &registry) {
    auto view = registry.view<const JointEnabledInput>();

    auto progress = registry.try_ctx<ProgressDialogControl>();
    auto progress_value = 1;
    auto max_progress = view.size();
    if (progress) {
        progress->control->reset();
        progress->control->message("Finding enabled joints...");
        progress->control->maximumValue(view.size());
    }

    for (auto &&[entity, enabled]: view.proxy()) {
        PLOG_DEBUG << "Updating JointEnabled value";
        registry.emplace<JointEnabled>(entity, enabled.control->value());

        if (progress) progress->control->progressValue(progress_value);
        progress_value += 1;
    }

    if (progress) progress->control->progressValue(max_progress);
}
