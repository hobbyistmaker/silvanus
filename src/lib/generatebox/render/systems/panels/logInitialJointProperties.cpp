//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/Dimensions.hpp"
#include "entities/Enabled.hpp"
#include "entities/JointEnabled.hpp"
#include "entities/JointDirection.hpp"
#include "entities/JointName.hpp"
#include "entities/JointOrientation.hpp"
#include "entities/JointPanelOffset.hpp"
#include "entities/JointPattern.hpp"
#include "entities/JointPatternDistance.hpp"
#include "entities/JointPosition.hpp"
#include "entities/JointThickness.hpp"
#include "entities/Panel.hpp"
#include "entities/PanelOffset.hpp"
#include "entities/PanelPosition.hpp"
#include "entities/Thickness.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void logInitialJointProperties(entt::registry &registry) {
    registry.view<
        Panel,
        Enabled,
        Dimensions,
        Thickness,
        PanelOffset,
        JointPanelOffset,
        JointPatternDistance,
        JointName,
        JointDirection,
        JointOrientation,
        PanelPosition,
        JointPosition,
        JointThickness,
        JointPattern,
        JointEnabled
    >().each([](
        auto entity,
        auto const &panel,
        auto const &enable,
        auto const &dimensions,
        auto const &thickness,
        auto const &panel_offset,
        auto const &joint_panel_offset,
        auto const &distance,
        auto const &joint_name,
        auto const &joint_direction,
        auto const &joint_orientation,
        auto const &panel_position,
        auto const &joint_position,
        auto const &joint_thickness,
        auto const &joint_pattern,
        auto const &joint_enabled
    ){
        PLOG_DEBUG << "<<<<<<<< " << panel.name << " jointed with " << joint_name.value << " <<<<<<<<";
        PLOG_DEBUG << " panel is enabled == " << enable.value;
        PLOG_DEBUG << " joint is enabled == " << joint_enabled.value;
        PLOG_DEBUG << " thickness is " << thickness.value;
        PLOG_DEBUG << " length is " << dimensions.length;
        PLOG_DEBUG << " width is " << dimensions.width;
        PLOG_DEBUG << " height is " << dimensions.height;
        PLOG_DEBUG << " orientation is " << (int)panel.orientation;
        PLOG_DEBUG << " panel offset == " << panel_offset.value;
        PLOG_DEBUG << " joint panel offset == " << joint_panel_offset.value;
        PLOG_DEBUG << " joint distance == " << distance.value << " along orientation " << (int)joint_orientation.axis;
        PLOG_DEBUG << " joint direction == " << (int)joint_direction.value;
        PLOG_DEBUG << " position is " << (int) panel_position.value;
        PLOG_DEBUG << " joint position is " << (int) joint_position.value;
        PLOG_DEBUG << " joint thickness is " << joint_thickness.value;
        PLOG_DEBUG << " joint pattern is " << (int) joint_pattern.value;
        PLOG_DEBUG << ">>>>>>>> " << panel.name << " jointed with " << joint_name.value << " >>>>>>>>";
    });
}