//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>

#include "entities/Dimensions.hpp"
#include "entities/JointExtrusion.hpp"
#include "entities/JointName.hpp"
#include "entities/JointThickness.hpp"

using namespace silvanus::generatebox::entities;

void initializeJointExtrusionFromThicknessOffsetAndName(entt::registry &registry) {
    auto create_extrusion_view = registry.view<const JointThickness, const JointPanelOffset, const JointName>().proxy();
    for (auto &&[entity, distance, offset, name]: create_extrusion_view) {
        PLOG_DEBUG << "Adding joint extrusion";
        registry.emplace<JointExtrusion>(
            entity, distance.value, offset.value, name.value
        );
    }
}