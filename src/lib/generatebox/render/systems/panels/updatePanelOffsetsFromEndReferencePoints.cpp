//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>

#include "entities/EndReferencePoint.hpp"
#include "entities/ExtrusionDistance.hpp"
#include "entities/OrientationTags.hpp"
#include "entities/PanelOffset.hpp"

using namespace silvanus::generatebox::entities;

void updatePanelOffsetsFromEndReferencePoints(entt::registry& registry) {
    PLOG_DEBUG << "Started updatePanelOffsets";
    auto length_view = registry.view<PanelOffset, const LengthOrientation, const EndReferencePoint, const ExtrusionDistance>().proxy();
    for (auto &&[entity, offset, orientation, end, distance]: length_view) {
        PLOG_DEBUG << "Adjusting length orientation panel offsets.";
        offset.value += (end.length.value - distance.value);
    }

    auto width_view = registry.view<PanelOffset, const WidthOrientation, const EndReferencePoint, const ExtrusionDistance>().proxy();
    for (auto &&[entity, offset, orientation, end, distance]: width_view) {
        PLOG_DEBUG << "Adjusting width orientation panel offsets.";
        offset.value += (end.width.value - distance.value);
    }

    auto height_view = registry.view<PanelOffset, const HeightOrientation, const EndReferencePoint, const ExtrusionDistance>().proxy();
    for (auto &&[entity, offset, orientation, end, distance]: height_view) {
        PLOG_DEBUG << "Adjusting height orientation panel offsets.";
        offset.value += (end.height.value - distance.value);
    }
    PLOG_DEBUG << "Finished updatePanelOffsets";
}