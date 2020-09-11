//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>

#include "entities/JointPanel.hpp"
#include "entities/Kerf.hpp"
#include "entities/PanelOffset.hpp"
#include "entities/PanelPosition.hpp"
#include "entities/Position.hpp"

using namespace silvanus::generatebox::entities;

void kerfAdjustOutsidePanelOffsetValues(entt::registry& registry) {
    auto outside_kerf_view = registry.view<PanelOffset, const PanelPosition, const Kerf, const KerfParam>().proxy();
    for (auto &&[entity, offset, position, kerf, param]: outside_kerf_view) {
        if (position.value == Position::Inside) continue;

        auto int_offset = (int)offset.value;

        if (int_offset == 0) continue;

        PLOG_DEBUG << "Adjusting outside panel offset kerf.";
        offset.value += kerf.value;

        offset.expression.shrink_to_fit();
        if (offset.expression.length() == 0) continue;
        offset.expression.append(" + " + param.expression);
        PLOG_DEBUG << "Adjusting outside panel offset kerf parameter: " << offset.expression;
    }
}

void kerfAdjustOutsidePanelOffsetExpressions(entt::registry& registry) {
    auto outside_kerf_view = registry.view<PanelOffsetParam, const PanelPosition, const KerfParam>().proxy();
    for (auto &&[entity, offset, position, kerf]: outside_kerf_view) {
        if (position.value == Position::Inside) continue;

        offset.expression.shrink_to_fit();

        if (offset.expression.length() == 0) continue;

        offset.expression.append(" + " + kerf.expression);
        PLOG_DEBUG << "Adjusting outside panel offset kerf parameter: " << offset.expression;
    }
}

void kerfAdjustOutsidePanelOffsets(entt::registry& registry) {
    kerfAdjustOutsidePanelOffsetValues(registry);
}