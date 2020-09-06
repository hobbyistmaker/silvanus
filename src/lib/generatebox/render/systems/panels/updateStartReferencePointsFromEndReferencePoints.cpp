//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>

#include "entities/EndReferencePoint.hpp"
#include "entities/ExtrusionDistance.hpp"
#include "entities/OrientationTags.hpp"
#include "entities/StartReferencePoint.hpp"

using namespace silvanus::generatebox::entities;

void updateStartReferencePointsFromEndReferencePoints(entt::registry& registry) {
    PLOG_DEBUG << "Started updateStartReferencePoints";

    auto length_view = registry.view<StartReferencePoint, const LengthOrientation, const EndReferencePoint, const ExtrusionDistance>().proxy();
    for (auto &&[entity, start, orientation, end, extrusion_distance]: length_view) {
        PLOG_DEBUG << "Updating start reference points for length orientation";
        auto length = end.length.value - extrusion_distance.value;

        start.length.value = length;
        start.width = end.width;
        start.height = end.height;
    }

    auto width_view = registry.view<StartReferencePoint, const WidthOrientation, const EndReferencePoint, const ExtrusionDistance>().proxy();
    for (auto &&[entity, start, orientation, end, extrusion_distance]: width_view) {
        PLOG_DEBUG << "Updating start reference points for width orientation";
        auto width = end.width.value - extrusion_distance.value;
        start.width.value = width;
        start.length = end.length;
        start.height = end.height;
    }

    auto height_view = registry.view<StartReferencePoint, const HeightOrientation, const EndReferencePoint, const ExtrusionDistance>().proxy();
    for (auto &&[entity, start, orientation, end, extrusion_distance]: height_view) {
        PLOG_DEBUG << "Updating start reference points for height orientation";

        auto height = end.height.value - extrusion_distance.value;
        start.height.value = height;
        start.length = end.length;
        start.width = end.width;
    }
    PLOG_DEBUG << "Finished updateStartReferencePoints";
}