//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>

#include "entities/JointExtrusion.hpp"
#include "entities/JointPatternTags.hpp"
#include "entities/Kerf.hpp"
#include "entities/PanelPosition.hpp"

using namespace silvanus::generatebox::entities;

void kerfAdjustInsideJointExtrusionExpressions(entt::registry &registry) {
    auto extrusion_view = registry.view<JointExtrusionParams, const PanelPosition, const KerfParam>().proxy();
    for (auto &&[entity, extrusion, position, kerf]: extrusion_view) {
        if (position.value == Position::Outside) continue;

        extrusion.offset.shrink_to_fit();

        if (extrusion.offset.length() == 0) continue;
        extrusion.offset.append(" + " + kerf.expression + "/2");
        PLOG_DEBUG << "Adjusting joint extrusion kerf: " << extrusion.offset;
    }
}

void kerfAdjustInsideJointExtrusionValues(entt::registry &registry) {
    auto extrusion_view = registry.view<JointExtrusion, const PanelPosition, const Kerf, const KerfParam>().proxy();
    for (auto &&[entity, extrusion, position, kerf, param]: extrusion_view) {
        if (position.value == Position::Outside) continue;

        if (static_cast<int>(extrusion.offset.value) == 0) continue;
        PLOG_DEBUG << "Adjusting joint extrusion kerf";
        extrusion.offset.value += kerf.value/2;

        extrusion.offset.expression.shrink_to_fit();
        if (extrusion.offset.expression.length() == 0) continue;
        extrusion.offset.expression.append(" + " + param.expression + "/2");
        PLOG_DEBUG << "Adjusting joint extrusion kerf: " << extrusion.offset.expression;
    }
}

void kerfAdjustInsideJointExtrusionOffset(entt::registry &registry) {
    kerfAdjustInsideJointExtrusionValues(registry);
}
