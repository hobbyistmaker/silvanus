//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/JointPatternValue.hpp"
#include "entities/Kerf.hpp"

#include <entt/entt.hpp>
#include <plog/Log.h>

using std::max;

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;

void kerfAdjustJointPatternValues(entt::registry &registry)  {
    auto kerf_view = registry.view<JointPatternValues, const Kerf>().proxy();
    for (auto &&[entity, values, kerf]: kerf_view) {
        values.finger_width -= kerf.value;
        values.pattern_offset += kerf.value;

        if (values.corner_width == 0) continue;

        PLOG_DEBUG << "Adjusting corner width kerf";
        values.corner_distance += kerf.value ;
        PLOG_DEBUG << "Corner width: " << values.corner_width;
        PLOG_DEBUG << "Corner distance: " << values.corner_distance;
        PLOG_DEBUG << "Kerf value: " << kerf.value;
    }
}