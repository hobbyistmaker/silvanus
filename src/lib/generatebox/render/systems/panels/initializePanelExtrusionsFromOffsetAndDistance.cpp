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
    PLOG_DEBUG << "Started initializePanelExtrusionsFromOffsetAndDistance";
    auto view = registry.view<Panel, const PanelOffset, const ExtrusionDistance>();
    for (auto &&[entity, panel, offset, distance]: view.proxy()) {
        PLOG_DEBUG << "Creating panel extrusions";
        registry.emplace_or_replace<PanelExtrusion>(
            entity, entity, distance, offset, panel.name
        );
    }
    PLOG_DEBUG << "Finished initializePanelExtrusionsFromOffsetAndDistance";

    PLOG_DEBUG << "Started initializePanelExtrusionParamsFromOffsetAndDistance";
    auto param_view = registry.view<PanelOffsetParam, ExtrusionDistanceParam>();
    for (auto &&[entity, offset, distance]: param_view.proxy()) {
        PLOG_DEBUG << "Creating panel extrusion parameters (distance, offset): " << distance.expression << ", " << offset.expression;
        registry.emplace_or_replace<PanelExtrusionParams>(
            entity, distance.expression, offset.expression
        );
    }
    PLOG_DEBUG << "Finished initializePanelExtrusionParamsFromOffsetAndDistance";
}

