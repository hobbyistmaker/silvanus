//
// Created by Hobbyist Maker on 9/6/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_UPDATEJOINTDIRECTION_HPP
#define SILVANUSPRO_UPDATEJOINTDIRECTION_HPP

#include "lib/generatebox/entities/DialogInputs.hpp"
#include "entities/JointDirection.hpp"
#include "entities/JointPosition.hpp"
#include "entities/PanelPosition.hpp"

#include <entt/entt.hpp>
#include <plog/Log.h>

using silvanus::generatebox::entities::DialogJointDirectionInputs;
using silvanus::generatebox::entities::DialogJointDirectionInput;

void updateJointDirectionImpl(entt::registry& registry);

template <class T, class P>
void updateInsideJointDirectionInputsImpl(entt::registry& registry, bool reverse = false) {
    PLOG_DEBUG << "starting updateInsideJointDirectionInputsImpl";
    auto view = registry.view<T, Panel, PanelPositions>();
    for (auto &&[entity, dest, panel, positions]: view.proxy()) {
        PLOG_DEBUG << panel.name << " positions are: " << (int)positions.first << ":" << (int)positions.second;
        if (positions.first == Position::Outside || positions.second == Position::Outside) continue;

        auto control = registry.ctx<P>().control;
        PLOG_DEBUG << "Adding Joint Direction Input to entity " << (int)entity << " with value: " << control->selectedItem()->index();
        registry.emplace<DialogJointDirectionInputs>(entity, DialogJointDirectionInput{control, reverse}, DialogJointDirectionInput{control, !reverse});
    }
}

template <class T>
void updateJointDirectionImpl(entt::registry& registry, Position panel_position, Position joint_position, JointDirectionType joint_direction) {
    auto view = registry.view<JointDirections, const PanelPositions, const JointPositions, const T>();
    PLOG_DEBUG << "Joint direction view size is " << view.size();
    for (auto &&[entity, directions, panels, joints, filter]: view.proxy()) {
        PLOG_DEBUG << "Matching panel position " << (int)panel_position << " to == " << (int)panels.first << ":" << (int)panels.second;
        PLOG_DEBUG << "Matching joint position " << (int)joint_position << " to == " << (int)joints.first << ":" << (int)joints.second;

        if (panels.first != panel_position || panels.second != joint_position) continue;

        PLOG_DEBUG << "Updating divider joint direction for entity " << (int)entity << " with direction " << (int)joint_direction;

        PLOG_DEBUG << "Starting directions == " << (int)directions.first << ":" << (int)directions.second;

        directions.first = (panels.first == panel_position && joints.first == joint_position) ? joint_direction : directions.first;
        directions.second = (panels.second == joint_position && joints.second == panel_position) ? static_cast<JointDirectionType>(!(bool)joint_direction) : directions.second;

        PLOG_DEBUG << "Ending directions == " << (int)directions.first << ":" << (int)directions.second;
    }
};

#endif //SILVANUSPRO_UPDATEJOINTDIRECTION_HPP
