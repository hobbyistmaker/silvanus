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

void kerfAdjustInsidePanelOffsetValues(entt::registry& registry) {
    auto inside_kerf_view = registry.view<PanelOffset, const PanelPosition, const Kerf, const KerfParam>().proxy();
    for (auto &&[entity, offset, position, kerf, param]: inside_kerf_view) {
        if (position.value == Position::Outside) continue;

        auto int_offset = static_cast<int>(offset.value);

        if (int_offset == 0) continue;

        PLOG_DEBUG << "Adjusting inside panel offset kerf.";
        offset.value += kerf.value/2;

        offset.expression.shrink_to_fit();
        if (offset.expression.length() == 0) continue;
        offset.expression.append(" + " + param.expression + "/2");
        PLOG_DEBUG << "Adjusting inside panel offset kerf: " << offset.expression;
    }
}

void kerfAdjustInsidePanelOffsetExpressions(entt::registry& registry) {
    auto inside_kerf_view = registry.view<PanelOffsetParam, const PanelPosition, const KerfParam>().proxy();
    for (auto &&[entity, offset, position, kerf]: inside_kerf_view) {
        if (position.value == Position::Outside) continue;

        offset.expression.shrink_to_fit();

        if (offset.expression.length() == 0) continue;

        offset.expression.append(" + " + kerf.expression + "/2");
        PLOG_DEBUG << "Adjusting inside panel offset kerf: " << offset.expression;
    }
}

void kerfAdjustInsidePanelOffsets(entt::registry& registry) {
    kerfAdjustInsidePanelOffsetValues(registry);
}