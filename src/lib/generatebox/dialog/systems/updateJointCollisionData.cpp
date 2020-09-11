//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "updateJointCollisionData.hpp"

#include "entities/DialogInputs.hpp"
#include "entities/Enabled.hpp"

#include "findJoints.hpp"

using namespace silvanus::generatebox::entities;

void updateJointCollisionDataImpl(entt::registry& registry) {
    auto view = registry.view<Enabled, DialogPanelCollisionData, JointPanels>().proxy();
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

    auto params_view = registry.view<DialogPanelCollisionDataParams, JointPanelsParams>();
    for (auto &&[entity, collision, joint]: params_view.proxy()) {
        PLOG_DEBUG << "Updating panel collision parameters for " << (int)entity;
        auto first_result = detectPanelCollisionsParamsImpl(joint.first, joint.second);
        collision.first.panel_offset = first_result.data.panel_offset;
        collision.first.joint_offset = first_result.data.joint_offset;
        collision.first.distance = first_result.data.distance;
        PLOG_DEBUG << "First PO: " << collision.first.panel_offset << " - JO: " << collision.first.joint_offset << " - DIST: " << collision.first.distance;

        auto second_result = detectPanelCollisionsParamsImpl(joint.second, joint.first);
        collision.second.panel_offset = second_result.data.panel_offset;
        collision.second.joint_offset = second_result.data.joint_offset;
        collision.second.distance = second_result.data.distance;
        PLOG_DEBUG << "Second PO: " << collision.second.panel_offset << " - JO: " << collision.second.joint_offset << " - DIST: " << collision.second.distance;
    }
}
