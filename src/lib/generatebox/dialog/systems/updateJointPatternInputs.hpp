//
// Created by Hobbyist Maker on 9/5/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_UPDATEJOINTPATTERNINPUTS_HPP
#define SILVANUSPRO_UPDATEJOINTPATTERNINPUTS_HPP

#include "entities/JointPattern.hpp"

#include <entt/entt.hpp>
#include <plog/Log.h>

using namespace silvanus::generatebox::entities;

template <class T, class P>
void updateJointPatternInputsImpl(entt::registry& registry, AxisFlag orientation) {
    auto joint_pattern_view = registry.view<PanelOrientation, JointOrientation, PanelPositions, T>().proxy();
    for (auto &&[entity, panel_orientation, joint_orientation, panel_positions, dest]: joint_pattern_view) {
        PLOG_DEBUG << "Checking Joint Pattern Inputs";
        if (panel_positions.first == Position::Inside && panel_positions.second == Position::Inside) continue;
        if (panel_positions.first == Position::Outside && panel_orientation.axis != orientation) continue;
        if (panel_positions.second == Position::Outside && joint_orientation.axis != orientation) continue;
        if (panel_orientation.axis != orientation && joint_orientation.axis != orientation) continue;

        auto control = registry.ctx<P>().control;
        PLOG_DEBUG << "Adding Joint Pattern Input to entity " << (int)entity << " with value: " << control->selectedItem()->index();
        registry.emplace<DialogJointPatternInput>(entity, control);
    }
}

#endif //SILVANUSPRO_UPDATEJOINTPATTERNINPUTS_HPP
