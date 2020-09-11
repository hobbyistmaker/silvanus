//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>

#include "entities/Dimensions.hpp"
#include "entities/JointExtrusion.hpp"
#include "entities/JointName.hpp"
#include "entities/JointPanelOffset.hpp"
#include "entities/JointThickness.hpp"

using namespace silvanus::generatebox::entities;

void initializeJointExtrusionValues(entt::registry& registry) {
    auto create_extrusion_view = registry.view<const JointThickness, const JointPanelOffset, const JointName>().proxy();
    for (auto &&[entity, distance, offset, name]: create_extrusion_view) {
        PLOG_DEBUG << (int)entity << "Adding joint extrusion values to " << name.value << ": " << distance.value << ", " << offset.value;
        PLOG_DEBUG << (int)entity << "Adding joint extrusion expressions to " << name.value << ": " << distance.expression << ", " << offset.expression;
        registry.emplace<JointExtrusion>(
            entity, entity, distance, offset, name.value
        );
    }
}

void initializeJointExtrusionExpressions(entt::registry& registry) {
    auto create_extrusion_view = registry.view<const JointThicknessParam, const JointPanelOffsetParam, const JointName>().proxy();
    for (auto &&[entity, distance, offset, name]: create_extrusion_view) {
        registry.emplace<JointExtrusionParams>(
            entity, distance.expression, offset.expression
        );
        PLOG_DEBUG << "Adding joint extrusion parameters to " << name.value << ": " << distance.expression << ", " << offset.expression;
    }
}

void initializeJointExtrusionFromThicknessOffsetAndName(entt::registry &registry) {
    initializeJointExtrusionExpressions(registry);
    initializeJointExtrusionValues(registry);
}