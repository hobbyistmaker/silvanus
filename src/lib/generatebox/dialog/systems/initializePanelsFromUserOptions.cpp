//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/EntitiesAll.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

#include "entity_helpers.hpp"

void logPanelThicknessParameters(entt::registry &panel_registry);
void logJointThicknessParameters(entt::registry &panel_registry);

using namespace silvanus::generatebox;
using namespace silvanus::generatebox::entities;

void initializePanelsFromUserOptionsImpl(entt::registry& configuration, entt::registry& panel_registry) {
    std::map<entt::entity, std::set<entt::entity>> first_index  = {};
    std::map<entt::entity, std::set<entt::entity>> second_index = {};
    std::map<entt::entity, entt::entity> primary_joint_index = {};
    std::map<entt::entity, entt::entity> secondary_joint_index = {};

    auto kerf = configuration.ctx<DialogKerfInput>().control->value();

    auto thickness_params_view = configuration.view<ThicknessParameter, const PanelThicknessActive>();
    for (auto &&[entity, parameter, thickness]: thickness_params_view.proxy()) {
        PLOG_DEBUG << "Initializing parameter " << parameter.name << " with value " << std::to_string(thickness.control->value());
        auto param_entity = panel_registry.create();
        panel_registry.emplace<FloatParameter>(param_entity, parameter.name, thickness.control->value(), thickness.control->expression(), thickness.control->unitType());
    }

    auto float_params_view = configuration.view<const FloatParameterInput>();
    for (auto &&[entity, parameter]: float_params_view.proxy()) {
        PLOG_DEBUG << "Initializing parameter " << parameter.name << " with value " << std::to_string(parameter.control->value());
        auto param_entity = panel_registry.create();
        panel_registry.emplace<FloatParameter>(param_entity, parameter.name, parameter.control->value(), parameter.control->expression(), parameter.control->unitType());
    }

    auto master_view = configuration
        .view<Enabled, FingerPattern, FingerWidth, JointPanels, DialogPanelCollisionData, DialogPanelCollisionDataParams, DialogPanels, JointPattern, PanelPositions, JointDirections>()
        .proxy();
    for (auto &&[en, enabled, fm, fw, joints, collision_data, collision_param, panels, pt, pp, jd]: master_view) {
        if (!enabled.value) continue;
        PLOG_DEBUG << "Joint Directions are " << (int) jd.first << ":" << (int) jd.second;

        auto create_panel = [&, entity = en, finger_mode = fm, finger_width = fw, pattern_type = pt](
            const JointPanelPlanes &joint, const DialogPanelJointData collision, DialogPanelJointDataParams params, const entt::entity &second_panel, const PanelPositions &positions,
            const JointDirectionType &joint_direction
        ) {
            auto panel_offset   = static_cast<int>(collision.panel_offset) == 0 ? 0.0 : collision.panel_offset;
            auto joint_distance = collision.distance;
            auto panel_position = positions.first;
            auto joint_position = positions.second;

            auto panel = makePanelEntity(panel_registry);
            panel_registry.emplace<Kerf>(panel, kerf);
            panel_registry.emplace<KerfParam>(panel, "kerf"); // TODO: Make this adjustable
            panel_registry.emplace<FingerPattern>(panel, finger_mode);
            panel_registry.emplace<FingerWidth>(panel, finger_width);
            panel_registry.emplace<FingerWidthParam>(panel, "finger_width"); // TODO: Make this adjustable
            panel_registry.emplace<JointPattern>(panel, pattern_type);
            panel_registry.emplace<JointPanelOffset>(panel, panel_offset);
            panel_registry.emplace<JointPanelOffsetParam>(panel, params.panel_offset);
            panel_registry.emplace<JointPatternDistance>(panel, joint_distance);
            panel_registry.emplace<JointPatternDistanceParam>(panel);

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

            PLOG_DEBUG << (int)entity << " to " << (int)panel << ":Adding panel registry entity for " << joint.panel.name;
            PLOG_DEBUG << joint.panel.name << " direction is " << (int) joint_direction;
            first_index[joint.entity].insert(panel);
            PLOG_DEBUG << joint.panel.name << " now has " << first_index[joint.entity].size() << " elements.";
            second_index[second_panel].insert(panel);
        };

        create_panel(joints.first, collision_data.first, collision_param.first, joints.second.entity, pp, jd.first);
        create_panel(joints.second, collision_data.second, collision_param.second, joints.first.entity, {pp.second, pp.first}, jd.second);
    }

    auto process_view = configuration.view<
        const PanelEnabled, const Panel, const PanelMaxPoint, const PanelMinPoint, const PanelAxis, const PanelThickness, const ThicknessParameter
    >();
    for (auto &&[entity, enable, panel_data, max_point, min_point, normal, thickness, thickness_param]: process_view.proxy()) {
        PLOG_DEBUG << (int)entity << "Generating panel configuration";
        auto first_panels  = first_index[entity];
        auto second_panels = second_index[entity];

        auto parent_panel = panel_registry.create();
        panel_registry.emplace<Panel>(parent_panel, panel_data.name, panel_data.priority, panel_data.orientation);
        panel_registry.emplace<ChildPanels>(parent_panel, first_panels);

        for (auto const &panel: first_panels) {
            PLOG_DEBUG << (int)entity << " to " << (int)panel << ": Adding enable, panel and dimension data to panel " << panel_data.name;
            PLOG_DEBUG << (int)entity << " to " << (int)panel << ": Setting thickness to " << std::to_string(thickness.value) << " (" << thickness_param.name << ")";
            panel_registry.emplace<PanelMaxPoint>(panel, max_point.length, max_point.width, max_point.height);
            panel_registry.emplace<PanelMinPoint>(panel, min_point.length, min_point.width, min_point.height);
            panel_registry.emplace<PanelAxis>(panel, normal.length, normal.width, normal.height);
            panel_registry.emplace<Enabled>(panel, enable.is_true);
            panel_registry.emplace<Panel>(panel, panel_data.name, panel_data.priority, panel_data.orientation);
            panel_registry.emplace<Thickness>(panel, thickness.value);
            panel_registry.emplace<PanelThicknessParameter>(panel, thickness_param.name, thickness_param.unit_type);
            panel_registry.emplace<ParentPanel>(panel, parent_panel);
        }

        for (auto const &panel: second_panels) {
            PLOG_DEBUG << (int)entity << " to " << (int)panel << ": Adding joint name for " << panel_data.name << " with thickness of " << thickness.value;
            panel_registry.emplace<JointEnabled>(panel, enable.is_true);
            panel_registry.emplace<JointName>(panel, panel_data.name);
            panel_registry.emplace<JointOrientation>(panel, panel_data.orientation);
            panel_registry.emplace<JointThickness>(panel, thickness.value, thickness_param.name);
            panel_registry.emplace<JointThicknessParameter>(panel, thickness_param.name, thickness_param.unit_type);
        }
    }

    auto param_view = configuration.view<const PanelMaxParam, const PanelMinParam>();
    for (auto &&[entity, max_param, min_param]: param_view.proxy()) {
        auto first_panels  = first_index[entity];

        for (auto const &panel_entity: first_panels) {
            panel_registry.emplace<PanelMaxParam>(panel_entity, max_param);
            panel_registry.emplace<PanelMinParam>(panel_entity, min_param);
            PLOG_DEBUG << (int)entity << " to " << (int)panel_entity << ": Adding max and min parameters to panel entity";
        }
    }

    logPanelThicknessParameters(panel_registry);
    logJointThicknessParameters(panel_registry);
}

void logJointThicknessParameters(entt::registry &panel_registry) {
    auto joint_thickness_view = panel_registry.view<JointThicknessParameter>();
    for (auto &&[entity, param]: joint_thickness_view.proxy()) {
        PLOG_DEBUG << "Found joint thickness parameter " << param.expression;
    }
}

void logPanelThicknessParameters(entt::registry &panel_registry) {
    auto thickness_view = panel_registry.view<PanelThicknessParameter>();
    for (auto &&[entity, param]: thickness_view.proxy()) {
        PLOG_DEBUG << "Found thickness parameter " << param.expression;
    }
}
