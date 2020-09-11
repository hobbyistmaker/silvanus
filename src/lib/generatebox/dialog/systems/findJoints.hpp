//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_FINDJOINTS_HPP
#define SILVANUSPRO_FINDJOINTS_HPP

#include "detectPanelCollisions.hpp"

#include "entities/EntitiesAll.hpp"
#include "lib/generatebox/entities/DialogInputs.hpp"
#include "entity_helpers.hpp"

#include <entt/entt.hpp>
#include <plog/Log.h>

using silvanus::generatebox::makeJointEntity;

using silvanus::generatebox::entities::AxisFlag;
using silvanus::generatebox::entities::DialogFingerMode;
using silvanus::generatebox::entities::DialogFingerWidthInput;
using silvanus::generatebox::entities::DialogFirstPlanes;
using silvanus::generatebox::entities::DialogFirstPlanesParams;
using silvanus::generatebox::entities::DialogJointIndex;
using silvanus::generatebox::entities::DialogJointPattern;
using silvanus::generatebox::entities::DialogJointPatternType;
using silvanus::generatebox::entities::DialogPanels;
using silvanus::generatebox::entities::DialogPanelCollisionData;
using silvanus::generatebox::entities::DialogPanelCollisionPair;
using silvanus::generatebox::entities::DialogPanelCollisionDataParams;
using silvanus::generatebox::entities::DialogPanelOverride;
using silvanus::generatebox::entities::DialogSecondPlanes;
using silvanus::generatebox::entities::DialogSecondPlanesParams;
using silvanus::generatebox::entities::DialogThicknessInput;
using silvanus::generatebox::entities::PanelPlanes;
using silvanus::generatebox::entities::PanelPlanesParams;
using silvanus::generatebox::entities::Enabled;
using silvanus::generatebox::entities::FingerPattern;
using silvanus::generatebox::entities::FingerWidthInput;
using silvanus::generatebox::entities::FingerWidth;
using silvanus::generatebox::entities::JointAxis;
using silvanus::generatebox::entities::JointDirections;
using silvanus::generatebox::entities::JointDirectionType;
using silvanus::generatebox::entities::JointOrientation;
using silvanus::generatebox::entities::JointPanels;
using silvanus::generatebox::entities::JointPanelsParams;
using silvanus::generatebox::entities::JointPanelPlanes;
using silvanus::generatebox::entities::JointPanelPlanesParams;
using silvanus::generatebox::entities::JointPattern;
using silvanus::generatebox::entities::JointPositions;
using silvanus::generatebox::entities::Panel;
using silvanus::generatebox::entities::PanelOrientation;
using silvanus::generatebox::entities::PanelPositions;
using silvanus::generatebox::entities::Position;

template <class T>
void findSecondaryPanelsImpl(
    entt::registry& registry,
    const JointPanelPlanes& first,
    const JointPanelPlanesParams& first_params,
    bool reverse = false
) {
    PLOG_DEBUG << "starting findSecondaryPanels";
    auto &index = registry.ctx<DialogJointIndex>();
    auto &finger_mode = registry.ctx<DialogFingerMode>();
    auto &finger_width = registry.ctx<DialogFingerWidthInput>();
    auto &default_thickness = registry.ctx<DialogThicknessInput>();

    auto view = registry.view<Panel, PanelPlanes, PanelPlanesParams>().proxy();
    for (auto &&[entity, panel, second_planes, second_planes_params]: view) {
        PLOG_DEBUG << "Found secondary panel " << panel.name;
        auto second = JointPanelPlanes{entity, panel, second_planes};
        auto second_params = JointPanelPlanesParams{entity, panel, second_planes_params};

        PLOG_DEBUG << "Checking if panel orientations are the same.";
        if (first.panel.orientation == second.panel.orientation) continue;

        auto first_result = detectPanelCollisionsImpl(first, second);
        auto enabled = first_result.collision_detected;

        PLOG_DEBUG << "Checking if " << panel.name << " panel is the primary.";
        if (!first_result.first_is_primary && !reverse)  continue;

        auto second_result = detectPanelCollisionsImpl(second, first);

        auto first_collision_params = detectPanelCollisionsParamsImpl(first_params, second_params);
        auto second_collision_params = detectPanelCollisionsParamsImpl(second_params, first_params);

        auto joint_entity = makeJointEntity(registry);
        PLOG_DEBUG << "Creating joint entity: " << (int)joint_entity << " for " << first.panel.name << " and " << second.panel.name;
        registry.emplace<T>(joint_entity);

        registry.emplace<Enabled>(joint_entity, enabled);
        registry.emplace<DialogFingerMode>(joint_entity, finger_mode);

        registry.emplace<DialogFirstPlanes>(joint_entity, first.planes);
        registry.emplace<DialogSecondPlanes>(joint_entity, second.planes);
        registry.emplace<DialogFirstPlanesParams>(joint_entity, first_params.planes);
        registry.emplace<DialogSecondPlanesParams>(joint_entity, second_params.planes);

        registry.emplace<DialogPanelCollisionData>(joint_entity, first_result.data, second_result.data);
        registry.emplace<DialogPanelCollisionDataParams>(joint_entity, first_collision_params.data, second_collision_params.data);

        registry.emplace<FingerWidthInput>(joint_entity, finger_width.control);
        registry.emplace<JointDirections>(joint_entity, JointDirectionType::Inverted, JointDirectionType::Normal);
        registry.emplace<JointAxis>(joint_entity, second.panel.axis.length, second.panel.axis.width, second.panel.axis.height);
        registry.emplace<JointOrientation>(joint_entity, second.panel.orientation);
        registry.emplace<JointPositions>(joint_entity, JointPositions{second_result.position, first_result.position});
        registry.emplace<Panel>(joint_entity, first.panel.name, first.panel.priority, first.panel.orientation);
        registry.emplace<PanelOrientation>(joint_entity, first.panel.orientation);
        registry.emplace<PanelPositions>(joint_entity, PanelPositions{first_result.position, second_result.position});

        auto panels = DialogPanels{first.entity, second.entity};
        registry.emplace<DialogPanels>(joint_entity, panels);

        PLOG_DEBUG << "Adding " << second.panel.name << " to list of second entities for " << first.panel.name;
        index.first_panels[first.entity].insert(joint_entity);
        index.second_panels[second.entity].insert(joint_entity);

        auto joint = JointPanels{first, second};
        auto joint_params = JointPanelsParams{first_params, second_params};
        registry.emplace<JointPanels>(joint_entity, joint);
        registry.emplace<JointPanelsParams>(joint_entity, joint_params);
        registry.emplace<DialogJointPattern>(joint_entity, first_result.pattern);
    }
    PLOG_DEBUG << "finished findSecondaryPanels";
}

template <class F1, class T>
void findJointsImpl(entt::registry& registry, bool reverse=false) {
    PLOG_DEBUG << "starting findJoints";
    auto first_entities = std::map<entt::entity, std::set<entt::entity>>{};
    auto second_entities = std::map<entt::entity, std::set<entt::entity>>{};
    registry.set<DialogJointIndex>(first_entities, second_entities);

    auto view = registry.view<Panel, PanelPlanes, PanelPlanesParams, F1>().proxy();
    for (auto &&[entity, panel, planes, params, filter]: view) {
        PLOG_DEBUG << "Finding joints for " << panel.name;
        findSecondaryPanelsImpl<T>(registry, {entity, panel, planes}, {entity, panel, params}, reverse);
    }
    PLOG_DEBUG << "finished findJoints";
}

#endif //SILVANUSPRO_FINDJOINTS_HPP
