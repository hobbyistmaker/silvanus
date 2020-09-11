//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/JointPatternValue.hpp"
#include "entities/Kerf.hpp"

#include <entt/entt.hpp>
#include <plog/Log.h>
#include <fmt/format.h>

using std::max;

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;

void kerfAdjustJointPatternValues(entt::registry &registry)  {
    auto kerf_view = registry.view<JointPatternValues, const Kerf>().proxy();
    for (auto &&[entity, values, kerf]: kerf_view) {
        values.finger_width -= kerf.value;
        values.pattern_offset += kerf.value;

        if (values.corner_width == 0) continue;

        PLOG_DEBUG << "Adjusting corner width kerf";
        values.corner_distance += kerf.value ;
        PLOG_DEBUG << "Corner width: " << values.corner_width;
        PLOG_DEBUG << "Corner distance: " << values.corner_distance;
        PLOG_DEBUG << "Kerf value: " << kerf.value;

    }

    // TODO: Add kerf adjustment to parameters
}

void kerfAdjustJointPatternExpressions(entt::registry& registry) {
    auto kerf_view = registry.view<JointPatternExpressions, const KerfParam>().proxy();
    for (auto &&[entity, expressions, kerf]: kerf_view) {
        expressions.finger_width = fmt::format("({} - {})", expressions.finger_width, kerf.expression);
        expressions.pattern_offset = fmt::format("({} + {})", expressions.pattern_offset, kerf.expression);

        if (expressions.corner_width.length() == 0) continue;

        PLOG_DEBUG << "Adjusting corner width kerf";
        expressions.corner_distance = fmt::format("({} + {})", expressions.corner_distance, kerf.expression);
        PLOG_DEBUG << "Corner width: " << expressions.corner_width;
        PLOG_DEBUG << "Corner distance: " << expressions.corner_distance;
        PLOG_DEBUG << "Kerf expression: " << kerf.expression;

    }
}