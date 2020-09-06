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

void kerfAdjustInsideJointExtrusionOffset(entt::registry &registry) {
    auto extrusion_view = registry.view<JointExtrusion, const PanelPosition, const Kerf>().proxy();
    for (auto &&[entity, extrusion, position, kerf]: extrusion_view) {
        if (position.value == Position::Outside) continue;

        if (static_cast<int>(extrusion.offset.value) == 0) continue;
        PLOG_DEBUG << "Adjusting joint extrusion kerf";
        extrusion.offset.value += kerf.value/2;
    }
}