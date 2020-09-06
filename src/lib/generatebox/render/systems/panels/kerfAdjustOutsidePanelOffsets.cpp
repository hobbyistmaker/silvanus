//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>

#include "entities/JointPanel.hpp"
#include "entities/Kerf.hpp"
#include "entities/PanelPosition.hpp"
#include "entities/Position.hpp"

using namespace silvanus::generatebox::entities;

void kerfAdjustOutsidePanelOffsets(entt::registry& registry) {
    auto outside_kerf_view = registry.view<PanelOffset, const PanelPosition, const Kerf>().proxy();
    for (auto &&[entity, offset, position, kerf]: outside_kerf_view) {
        if (position.value == Position::Inside) continue;

        auto int_offset = (int)offset.value;

        if (int_offset == 0) continue;

        PLOG_DEBUG << "Adjusting outside panel offset kerf.";
        offset.value += kerf.value;
    }
}