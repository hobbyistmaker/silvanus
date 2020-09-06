//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/AxisFlag.hpp"
#include "entities/OrientationTags.hpp"
#include "entities/Panel.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void tagHeightOrientationPanels(entt::registry &registry) {
    auto panel_height_view = registry.view<const Panel>().proxy();
    for (auto &&[entity, panel]: panel_height_view) {
        if (panel.orientation != AxisFlag::Height) continue;
        PLOG_DEBUG << "Add height orientation for " << panel.name;
        registry.emplace<HeightOrientation>(entity);
    }
}

void tagWidthOrientationPanels(entt::registry &registry) {
    auto panel_width_view = registry.view<const Panel>().proxy();
    for (auto &&[entity, panel]: panel_width_view) {
        if (panel.orientation != AxisFlag::Width) continue;
        PLOG_DEBUG << "Add width orientation for " << panel.name;
        registry.emplace<WidthOrientation>(entity);
    }
}

void tagLengthOrientationPanels(entt::registry &registry) {
    auto panel_length_view = registry.view<const Panel>().proxy();
    for (auto &&[entity, panel]: panel_length_view) {
        if (panel.orientation != AxisFlag::Length) continue;
        PLOG_DEBUG << "Add length orientation for " << panel.name;
        registry.emplace<LengthOrientation>(entity);
    }
}