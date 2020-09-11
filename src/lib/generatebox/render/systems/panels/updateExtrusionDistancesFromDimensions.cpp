//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>

#include "entities/ExtrusionDistance.hpp"
#include "entities/Thickness.hpp"

using namespace silvanus::generatebox::entities;

void updateExtrusionDistanceValues(entt::registry& registry) {
    auto value_view = registry.view<ExtrusionDistance, const Thickness, const PanelThicknessParameter>();
    for (auto &&[entity, distance, thickness, param]: value_view.proxy()) {
        PLOG_DEBUG << "Updating extrusion distance to " << thickness.value << " and " << param.expression;
        distance.value = thickness.value;
        distance.expression = param.expression;
    }
}

void updateExtrusionDistancesFromDimensions(entt::registry& registry) {
    PLOG_DEBUG << "Started updateExtrusionDistances";
    updateExtrusionDistanceValues(registry);
    PLOG_DEBUG << "Finished updateExtrusionDistances";
}