//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "DialogSystemManager.hpp"

#include "entities/Enabled.hpp"

#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void updateJointCollisionDataImpl(entt::registry& registry) {
    auto view = registry.view<Enabled, DialogPanelCollisionData, DialogJoints>().proxy();
    for (auto &&[entity, enabled, collision, joint]: view) {
        auto first_result = detectPanelCollisionsImpl(joint.first, joint.second);
        enabled.value = (first_result.collision_detected && first_result.first_is_primary);
        collision.first.panel_offset = first_result.data.panel_offset;
        collision.first.joint_offset = first_result.data.joint_offset;
        collision.first.distance = first_result.data.distance;

        auto second_result = detectPanelCollisionsImpl(joint.second, joint.first);
        collision.second.panel_offset = second_result.data.panel_offset;
        collision.second.joint_offset = second_result.data.joint_offset;
        collision.second.distance = second_result.data.distance;
    }
}