//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>

#include "entities/Panel.hpp"
#include "entities/PanelGroup.hpp"
#include "entities/PanelPosition.hpp"

using namespace silvanus::generatebox::entities;

void initializePanelGroupFromProfileOrientationAndPosition(entt::registry& registry) {
    PLOG_DEBUG << "Started addPanelGroups";
    auto view = registry.view<Panel, PanelProfile, ExtrusionDistance, PanelPosition>().proxy();
    for (auto &&[entity, panel, profile, distance, position]: view) {
        PLOG_DEBUG << "Creating panel group";
        registry.emplace_or_replace<PanelGroup>(
            entity, panel.orientation, profile, distance, position.value
        );
    }
    PLOG_DEBUG << "Finished addPanelGroups";
}
