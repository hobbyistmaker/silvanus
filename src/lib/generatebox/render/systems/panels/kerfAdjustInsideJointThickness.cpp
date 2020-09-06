//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>

#include "entities/JointPatternTags.hpp"
#include "entities/JointPosition.hpp"
#include "entities/JointThickness.hpp"
#include "entities/Kerf.hpp"

using namespace silvanus::generatebox::entities;

void kerfAdjustInsideJointThickness(entt::registry& registry) {
    auto thickness_view = registry.view<JointThickness, const Kerf, const JointPosition>();
    for (auto &&[entity, thickness, kerf, position]: thickness_view.proxy()) {
        PLOG_DEBUG << "Checking joint thickness for kerf adjustment";
        if (position.value == Position::Outside) continue;

        PLOG_DEBUG << "Adjusting joint thickness";
        thickness.value -= kerf.value;
    }
}