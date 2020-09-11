//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>

#include "entities/JointPanel.hpp"
#include "entities/JointPanelOffset.hpp"
#include "entities/Kerf.hpp"
#include "entities/PanelPosition.hpp"
#include "entities/Position.hpp"

using namespace silvanus::generatebox::entities;

void kerfAdjustOutsideJointPanelOffsetValues(entt::registry& registry) {
    auto kerf_joint_view = registry.view<JointPanelOffset, const PanelPosition, const Kerf, const KerfParam>().proxy();
    for (auto &&[entity, offset, position, kerf, param]: kerf_joint_view) {
        if (position.value == Position::Inside) continue;

        auto int_offset = static_cast<int>(offset.value);

        if (int_offset == 0) continue;

        PLOG_DEBUG << "Adjusting outside joint panel offset kerf.";
        offset.value += kerf.value;

        offset.expression.shrink_to_fit();
        if (offset.expression.length() == 0) continue;
        offset.expression.append(" + " + param.expression);
        PLOG_DEBUG << "Adjusting outside joint panel offset kerf expression: " << offset.expression;
    }
}

void kerfAdjustOutsideJointPanelOffsetExpressions(entt::registry& registry) {
    auto kerf_joint_view = registry.view<JointPanelOffsetParam, const PanelPosition, const KerfParam>().proxy();
    for (auto &&[entity, offset, position, kerf]: kerf_joint_view) {
        if (position.value == Position::Inside) continue;

        offset.expression.shrink_to_fit();

        if (offset.expression.length() == 0) continue;

        offset.expression.append(" + " + kerf.expression);
        PLOG_DEBUG << "Adjusting outside joint panel offset kerf expression: " << offset.expression;
    }
}

void kerfAdjustOutsideJointPanelOffsets(entt::registry& registry) {
    kerfAdjustOutsideJointPanelOffsetValues(registry);
}
