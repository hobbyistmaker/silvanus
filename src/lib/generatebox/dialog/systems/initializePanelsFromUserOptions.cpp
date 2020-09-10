//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/AxisFlag.hpp"
#include "entities/ChildPanels.hpp"
#include "entities/Dimensions.hpp"
#include "entities/Enabled.hpp"
#include "entities/EndReferencePoint.hpp"
#include "entities/ExtrusionDistance.hpp"
#include "entities/FingerWidth.hpp"
#include "entities/FingerPattern.hpp"
#include "entities/JointEnabled.hpp"
#include "entities/JointDirection.hpp"
#include "entities/JointName.hpp"
#include "entities/JointOrientation.hpp"
#include "entities/JointPattern.hpp"
#include "entities/JointPatternDistance.hpp"
#include "entities/JointPosition.hpp"
#include "entities/JointProfile.hpp"
#include "entities/JointThickness.hpp"
#include "entities/Kerf.hpp"
#include "entities/MaxOffset.hpp"
#include "entities/Panel.hpp"
#include "entities/PanelDimension.hpp"
#include "entities/PanelOffset.hpp"
#include "entities/PanelPosition.hpp"
#include "entities/PanelProfile.hpp"
#include "entities/ParentPanel.hpp"
#include "entities/StartReferencePoint.hpp"
#include "entities/Thickness.hpp"
#include "entities/JointPatternPosition.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void initializePanelsFromUserOptionsImpl(entt::registry& configuration, entt::registry& panel_registry) {
    std::map<entt::entity, std::set<entt::entity>> first_index  = {};
    std::map<entt::entity, std::set<entt::entity>> second_index = {};

    auto kerf = configuration.ctx<DialogKerfInput>().control->value();

    auto master_view = configuration
        .view<Enabled, FingerPattern, FingerWidth, DialogJoints, DialogPanelCollisionData, DialogPanels, JointPattern, PanelPositions, JointDirections>()
        .proxy();
    for (auto &&[entity, enabled, fm, fw, joints, collision_data, panels, pt, pp, jd]: master_view) {
        PLOG_DEBUG << "Joint Directions are " << (int) jd.first << ":" << (int) jd.second;
        auto create_panel = [&, finger_mode = fm, finger_width = fw, pattern_type = pt](
            const DialogPanelJoint &joint, const DialogPanelJointData collision, const entt::entity &second_panel, const PanelPositions &positions,
            const JointDirectionType &joint_direction
        ) {
            auto panel_offset   = static_cast<int>(collision.panel_offset) == 0 ? 0.0 : collision.panel_offset;
            auto joint_distance = collision.distance;
            auto panel_position = positions.first;
            auto joint_position = positions.second;

            auto panel = panel_registry.create();
            panel_registry.emplace<Kerf>(panel, kerf);
            panel_registry.emplace<FingerPattern>(panel, finger_mode);
            panel_registry.emplace<FingerWidth>(panel, finger_width);
            panel_registry.emplace<JointPattern>(panel, pattern_type);
            panel_registry.emplace<JointPanelOffset>(panel, panel_offset);
            panel_registry.emplace<JointPatternDistance>(panel, joint_distance);

            panel_registry.emplace<PanelPosition>(panel, panel_position);
            panel_registry.emplace<JointPosition>(panel, joint_position);
            panel_registry.emplace<JointProfile>(
                panel, panel_position, joint_position, joint_direction, JointPatternType::BoxJoint, FingerPatternType::AutomaticWidth, 0, 0.0, 0.0, 0.0, 0.0,
                AxisFlag::Length, AxisFlag::Length
            );
            panel_registry.emplace<JointPatternPosition>(
                panel, panel_position, AxisFlag::Length, JointPatternType::BoxJoint, AxisFlag::Length, joint_position
            );
            panel_registry.emplace<JointDirection>(panel, joint_direction);

            panel_registry.emplace<PanelOffset>(panel);
            panel_registry.emplace<EndReferencePoint>(panel);
            panel_registry.emplace<ExtrusionDistance>(panel);
            panel_registry.emplace<PanelProfile>(panel);
            panel_registry.emplace<StartReferencePoint>(panel);

            PLOG_DEBUG << "Adding panel registry entity for " << joint.panel.name;
            PLOG_DEBUG << joint.panel.name << " direction is " << (int) joint_direction;
            first_index[joint.entity].insert(panel);
            PLOG_DEBUG << joint.panel.name << " now has " << first_index[joint.entity].size() << " elements.";
            second_index[second_panel].insert(panel);
        };

        create_panel(joints.first, collision_data.first, joints.second.entity, pp, jd.first);
        create_panel(joints.second, collision_data.second, joints.first.entity, {pp.second, pp.first}, jd.second);
    }

    auto process_view = configuration.view<
        const DialogPanelEnableValue, const DialogPanel, const PanelDimensions, const DialogPanelThickness
    >().proxy();
    for (auto &&[entity, enable, panel_data, dimensions, thickness]: process_view) {
        PLOG_DEBUG << "Generating panel configuration";
        auto first_panels  = first_index[entity];
        auto second_panels = second_index[entity];

        auto parent_panel = panel_registry.create();
        panel_registry.emplace<Panel>(parent_panel, panel_data.name, panel_data.priority, panel_data.orientation);
        panel_registry.emplace<ChildPanels>(parent_panel, first_panels);

        for (auto const &panel: first_panels) {
            PLOG_DEBUG << "Adding enable, panel and dimension data to panel " << panel_data.name;
            PLOG_DEBUG << "Setting thickness to " << std::to_string(thickness.control->value());
            panel_registry.emplace<Dimensions>(panel, dimensions.length, dimensions.width, dimensions.height, thickness.control->value());
            panel_registry.emplace<Enabled>(panel, enable.value);
            panel_registry.emplace<Panel>(panel, panel_data.name, panel_data.priority, panel_data.orientation);
            panel_registry.emplace<Thickness>(panel, thickness.control->value());
            panel_registry.emplace<ParentPanel>(panel, parent_panel);
        }

        for (auto const &panel: second_panels) {
            auto joint_thickness = thickness.control->value();
            PLOG_DEBUG << "Adding joint name for " << panel_data.name << " with thickness of " << joint_thickness;
            panel_registry.emplace<JointEnabled>(panel, enable.value);
            panel_registry.emplace<JointName>(panel, panel_data.name);
            panel_registry.emplace<JointOrientation>(panel, panel_data.orientation);
            panel_registry.emplace<JointThickness>(panel, joint_thickness);
        }
    }

    auto length_offset_view = configuration.view<const LengthMaxOffsetInput>();
    for (auto &&[entity, max_offset]: length_offset_view.proxy()) {
        auto first_panels  = first_index[entity];
        for (auto const &panel: first_panels) {
            panel_registry.emplace<MaxOffset>(panel, max_offset.control->value());
        }
    };

    auto width_offset_view = configuration.view<const WidthMaxOffsetInput>();
    for (auto &&[entity, max_offset]: width_offset_view.proxy()) {
        auto first_panels  = first_index[entity];
        for (auto const &panel: first_panels) {
            panel_registry.emplace<MaxOffset>(panel, max_offset.control->value());
        }
    };

    auto height_offset_view = configuration.view<const HeightMaxOffsetInput>();
    for (auto &&[entity, max_offset]: height_offset_view.proxy()) {
        auto first_panels  = first_index[entity];
        for (auto const &panel: first_panels) {
            panel_registry.emplace<MaxOffset>(panel, max_offset.control->value());
        }
    };
}