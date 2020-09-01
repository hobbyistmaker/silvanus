//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_FINDJOINTS_HPP
#define SILVANUSPRO_FINDJOINTS_HPP

#include "entities/AxisFlag.hpp"
#include "entities/DialogInputs.hpp"
#include "entities/Enabled.hpp"
#include "entities/FingerPattern.hpp"
#include "entities/FingerWidth.hpp"
#include "entities/FingerWidthInput.hpp"
#include "entities/JointDirection.hpp"
#include "entities/JointPattern.hpp"
#include "entities/JointPosition.hpp"
#include "entities/Panel.hpp"
#include "entities/PanelPosition.hpp"
#include "entities/Position.hpp"

#include <entt/entt.hpp>
#include <plog/Log.h>

using silvanus::generatebox::entities::AxisFlag;
using silvanus::generatebox::entities::DialogFingerMode;
using silvanus::generatebox::entities::DialogFingerWidthInput;
using silvanus::generatebox::entities::DialogFirstPlanes;
using silvanus::generatebox::entities::DialogJointIndex;
using silvanus::generatebox::entities::DialogJointPattern;
using silvanus::generatebox::entities::DialogJointPatternType;
using silvanus::generatebox::entities::DialogJoints;
using silvanus::generatebox::entities::DialogPanel;
using silvanus::generatebox::entities::DialogPanelCollisionData;
using silvanus::generatebox::entities::DialogPanelCollisionPair;
using silvanus::generatebox::entities::DialogPanelJoint;
using silvanus::generatebox::entities::DialogPanelPlanes;
using silvanus::generatebox::entities::DialogPanels;
using silvanus::generatebox::entities::DialogSecondPlanes;
using silvanus::generatebox::entities::Enabled;
using silvanus::generatebox::entities::FingerPattern;
using silvanus::generatebox::entities::FingerWidthInput;
using silvanus::generatebox::entities::FingerWidth;
using silvanus::generatebox::entities::JointPattern;
using silvanus::generatebox::entities::JointPositions;
using silvanus::generatebox::entities::JointDirections;
using silvanus::generatebox::entities::Panel;
using silvanus::generatebox::entities::PanelPositions;
using silvanus::generatebox::entities::Position;

auto detectPanelCollisionsImpl(const DialogPanelJoint &first, const DialogPanelJoint &second) -> DialogPanelCollisionPair;

template <class F2, class T>
void findSecondaryPanelsImpl(
    entt::registry& registry,
    const DialogPanelJoint first
) {
    PLOG_DEBUG << "starting findSecondaryPanels";
    auto &index = registry.ctx<DialogJointIndex>();
    auto &finger_mode = registry.ctx<DialogFingerMode>();
    auto &finger_width = registry.ctx<DialogFingerWidthInput>();

    auto view = registry.view<DialogPanel, DialogPanelPlanes, F2>().proxy();
    for (auto &&[entity, panel, second_planes, filter]: view) {
        auto second = DialogPanelJoint{entity, panel, second_planes};

        PLOG_DEBUG << "Checking if panel orientations are the same.";
        if (first.panel.orientation == second.panel.orientation) continue;

        auto first_result = detectPanelCollisionsImpl(first, second);
        auto enabled = first_result.collision_detected;

        PLOG_DEBUG << "Checking if " << panel.name << " panel is the primary.";
        if (!first_result.first_is_primary) continue;

        auto second_result = detectPanelCollisionsImpl(second, first);

        auto joint_entity = registry.create();
        PLOG_DEBUG << "Creating joint entity: " << (int)joint_entity << " for " << first.panel.name << " and " << second.panel.name;
        registry.emplace<T>(joint_entity);
        registry.emplace<Enabled>(joint_entity, enabled);
        registry.emplace<DialogPanelCollisionData>(joint_entity, first_result.data, second_result.data);
        registry.emplace<DialogFirstPlanes>(joint_entity, first.planes);
        registry.emplace<DialogSecondPlanes>(joint_entity, second.planes);
        registry.emplace<PanelPositions>(joint_entity, PanelPositions{first_result.position, second_result.position});
        registry.emplace<JointPositions>(joint_entity, JointPositions{second_result.position, first_result.position});
        registry.emplace<DialogFingerMode>(joint_entity, finger_mode);
        registry.emplace<FingerPattern>(joint_entity);
        registry.emplace<JointPattern>(joint_entity);
        registry.emplace<JointDirections>(joint_entity);
        registry.emplace<FingerWidthInput>(joint_entity, finger_width.control);
        registry.emplace<FingerWidth>(joint_entity);
        registry.emplace<Panel>(joint_entity, panel.name, panel.priority, panel.orientation);

        auto panels = DialogPanels{first.entity, second.entity};
        registry.emplace<DialogPanels>(joint_entity, panels);

        PLOG_DEBUG << "Adding " << second.panel.name << " to list of second entities for " << first.panel.name;
        index.first_panels[first.entity].insert(joint_entity);
        index.second_panels[second.entity].insert(joint_entity);

        auto joint = DialogJoints{first, second};
        registry.emplace<DialogJoints>(joint_entity, joint);
        registry.emplace<DialogJointPattern>(joint_entity, first_result.pattern);
    }
    PLOG_DEBUG << "finished findSecondaryPanels";
}

template <class F1, class F2, class T>
void findJointsImpl(entt::registry& registry) {
    PLOG_DEBUG << "starting findJoints";
    auto first_entities = std::map<entt::entity, std::set<entt::entity>>{};
    auto second_entities = std::map<entt::entity, std::set<entt::entity>>{};
    registry.set<DialogJointIndex>(first_entities, second_entities);

    auto view = registry.view<DialogPanel, DialogPanelPlanes, F1>().proxy();
    for (auto &&[entity, panel, planes, filter]: view) {
        PLOG_DEBUG << "Pulling panel information.";
        auto const name         = panel.name;
        auto const orientation  = panel.orientation;
        auto const& length      = planes.length;
        auto const& width       = planes.width;
        auto const& height      = planes.height;

        PLOG_DEBUG << name << " length plane:  (" << planes.length.min_x << ", " << planes.length.min_y << ") to (" << planes.length.max_x << ", " << planes.length.max_y << ")";
        PLOG_DEBUG << name << " width plane :  (" << planes.width.min_x << ", " << planes.width.min_y << ") to (" << planes.width.max_x << ", " << planes.width.max_y << ")";
        PLOG_DEBUG << name << " height plane:  (" << planes.height.min_x << ", " << planes.height.min_y << ") to (" << planes.height.max_x << ", " << planes.height.max_y << ")";

        findSecondaryPanelsImpl<F2, T>(registry, {entity, panel, planes});
    }
    PLOG_DEBUG << "finished findJoints";
}

#endif //SILVANUSPRO_FINDJOINTS_HPP
