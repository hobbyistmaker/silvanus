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

void kerfAdjustInsideJointThicknessValues(entt::registry& registry) {
    auto thickness_view = registry.view<JointThickness, const Kerf, const KerfParam, const JointPosition>();
    for (auto &&[entity, thickness, kerf, param, position]: thickness_view.proxy()) {
        PLOG_DEBUG << "Checking joint thickness for kerf adjustment";
        if (position.value == Position::Outside) continue;

        PLOG_DEBUG << "Adjusting joint thickness";
        thickness.value -= kerf.value;

        thickness.expression = thickness.expression + " - " + param.expression;
        PLOG_DEBUG << "Kerf adjusting joint thickness parameter " << thickness.expression;
    }
}

void kerfAdjustInsideJointThicknessExpressions(entt::registry& registry) {
    auto thickness_view = registry.view<JointThicknessParam, const KerfParam, const JointPosition>();
    for (auto &&[entity, thickness, kerf, position]: thickness_view.proxy()) {
        PLOG_DEBUG << "Checking joint thickness for kerf adjustment";
        if (position.value == Position::Outside) continue;

        thickness.expression = thickness.expression + " - " + kerf.expression;
        PLOG_DEBUG << "Kerf adjusting joint thickness parameter " << thickness.expression;
    }
}

void kerfAdjustInsideJointThickness(entt::registry& registry) {
    kerfAdjustInsideJointThicknessValues(registry);
}