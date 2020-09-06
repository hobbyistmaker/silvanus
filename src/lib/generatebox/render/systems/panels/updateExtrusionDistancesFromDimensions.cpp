//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>

#include "entities/Dimensions.hpp"
#include "entities/ExtrusionDistance.hpp"
#include "entities/JointName.hpp"

using namespace silvanus::generatebox::entities;

void updateExtrusionDistancesFromDimensions(entt::registry& registry) {
    PLOG_DEBUG << "Started updateExtrusionDistances";
    auto view = registry.view<ExtrusionDistance, const Dimensions>().proxy();

    for (auto &&[entity, distance, dimensions]: view) {
        PLOG_DEBUG << "Updating extrusion distance to " << std::to_string(dimensions.thickness);
        distance.value = dimensions.thickness;
    }
    PLOG_DEBUG << "Finished updateExtrusionDistances";
}