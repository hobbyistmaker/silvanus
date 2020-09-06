//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/JointPatternPosition.hpp"
#include "entities/JointPosition.hpp"
#include "entities/PanelPosition.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void updateJointPatternPositionsFromPanelAndJointPositions(entt::registry &registry) {
    auto pattern_position_view = registry.view<JointPatternPosition, const PanelPosition, const JointPosition>().proxy();
    for (auto &&[entity, pattern, panel, joint]: pattern_position_view) {
        PLOG_DEBUG << "Updating joint pattern position with panel and joint position";
        pattern.panel_position = panel.value;
        pattern.joint_position = joint.value;
    }
}
