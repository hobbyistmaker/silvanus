//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>

#include "entities/ExtrusionDistance.hpp"
#include "entities/JointPanelOffset.hpp"
#include "entities/OrientationTags.hpp"
#include "entities/PanelMinPoint.hpp"
#include "entities/PanelAxis.hpp"
#include "entities/PanelOffset.hpp"

using namespace silvanus::generatebox::entities;

void updatePanelOffsetsValues(entt::registry& registry) {
    auto view = registry.view<PanelOffset, const PanelMinPoint, const PanelMinParam, const PanelAxis>();

    for (auto &&[entity, offset, min_point, min_param, normal]: view.proxy()) {
        auto length = min_point.length * normal.length;
        auto width = min_point.width * normal.width;
        auto height = min_point.height * normal.height;

        offset.value = std::max({length, width, height});

        auto length_param = normal.length ? min_param.length : "";
        auto width_param = normal.width ? min_param.width : "";
        auto height_param = normal.height ? min_param.height : "";

        offset.expression.shrink_to_fit();

        offset.expression = length_param.append(width_param).append(height_param);
        PLOG_DEBUG << (int)entity << ": Adjusting panel offset value to " << offset.value;
    }
}

void updatePanelOffsetsExpressions(entt::registry& registry) {
    auto view = registry.view<PanelOffsetParam, const PanelMinParam, const PanelAxis>();

    for (auto &&[entity, offset, min_point, normal]: view.proxy()) {
        auto length = normal.length ? min_point.length : "";
        auto width = normal.width ? min_point.width : "";
        auto height = normal.height ? min_point.height : "";

        offset.expression.shrink_to_fit();

        offset.expression = length.append(width).append(height);
        PLOG_DEBUG << (int)entity << ": Adjusting panel offset expression to " << offset.expression;
    }
}

void updatePanelOffsetsFromPanelMinPoints(entt::registry& registry) {
    PLOG_DEBUG << "Started updatePanelOffsetsFromPanelMinPoints";
    updatePanelOffsetsValues(registry);
    updatePanelOffsetsExpressions(registry);
    PLOG_DEBUG << "Finished updatePanelOffsetsFromPanelMinPoints";
}