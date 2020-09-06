//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>

#include "entities/ExtrusionDistance.hpp"
#include "entities/Panel.hpp"
#include "entities/PanelExtrusion.hpp"
#include "entities/PanelOffset.hpp"

using namespace silvanus::generatebox::entities;

void initializePanelExtrusionsFromOffsetAndDistance(entt::registry& registry) {
    PLOG_DEBUG << "Started addPanelExtrusions";
    auto view = registry.view<Panel, PanelOffset, ExtrusionDistance>().proxy();
    for (auto &&[entity, panel, offset, distance]: view) {
        PLOG_DEBUG << "Creating panel extrusions";
        registry.emplace_or_replace<PanelExtrusion>(
            entity, distance, offset, panel.name
        );
    }
    PLOG_DEBUG << "Finished addPanelExtrusions";
}

