//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/28/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "ConfigureJoints.hpp"
#include "entities/AxisFlag.hpp"
#include "entities/ChildPanels.hpp"
#include "entities/Dimensions.hpp"
#include "entities/Enabled.hpp"
#include "entities/EndReferencePoint.hpp"
#include "entities/FingerWidth.hpp"
#include "entities/FingerPattern.hpp"
#include "entities/InsidePanel.hpp"
#include "entities/JoinedPanels.hpp"
#include "entities/JointDirection.hpp"
#include "entities/JointGroup.hpp"
#include "entities/JointOrientation.hpp"
#include "entities/JointPatternDistance.hpp"
#include "entities/JointPatternPosition.hpp"
#include "entities/JointPatternTags.hpp"
#include "entities/JointPatternValue.hpp"
#include "entities/JointProfile.hpp"
#include "entities/JointProfileGroup.hpp"
#include "entities/JointThickness.hpp"
#include "entities/Kerf.hpp"
#include "entities/OrientationGroup.hpp"
#include "entities/OutsidePanel.hpp"
#include "entities/Panel.hpp"
#include "entities/PanelOrientation.hpp"
#include "entities/ParentPanel.hpp"
#include "entities/Position.hpp"

#include <algorithm>
#include <map>
#include <unordered_map>
#include <set>

#include "plog/Log.h"

using std::max;

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::systems;

void ConfigureJoints::execute() {


    updateJointPatternDistances();
    addFingerParameters();
    updateJointProfiles();
    updateJointProfileGroups();
    addJointGroups();

}

void ConfigureJoints::updateJointPatternDistances() {
    PLOG_DEBUG << "Started updateJointPatternDistances";
    auto view = m_registry.view<JointPatternDistance, const OrientationGroup, const EndReferencePoint>().proxy();
    for (auto &&[entity, pattern_distance, orientation, reference]: view) {
        PLOG_DEBUG << "Updating Joint Pattern Distance: " << (int)orientation.finger << ":" << (int)orientation.panel;
        auto const& distance = reference_selector[orientation.panel][orientation.finger](reference);
        pattern_distance.value = distance.value;
    }
    PLOG_DEBUG << "Finished updateJointPatternDistances";
}

void ConfigureJoints::addFingerParameters() {
    PLOG_DEBUG << "Started addFingerParameters";
    auto normal_automatic_view = m_registry.view<Panel, NormalJointDirection, BoxJointPattern, FingerWidth, JointPatternDistance, AutomaticFingerPatternType>().proxy();
    for (auto &&[entity, panel, direction, pattern, finger_width, pattern_distance, pattern_type]: normal_automatic_view) {
        PLOG_DEBUG << "Adding normal joint finger patterns to " << panel.name;
        auto length = pattern_distance.value;
        auto width = finger_width.value;

        auto default_fingers = ceil(length / width);
        auto estimated_fingers = max(5.0, (floor(default_fingers / 2) * 2) - 1);
        auto actual_finger_width = length / estimated_fingers;
        auto pattern_offset = actual_finger_width;
        auto actual_number_fingers = floor(estimated_fingers / 2);
        auto distance = (estimated_fingers - 3) * actual_finger_width;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width*2, distance, pattern_offset
        );
    }

    auto inverse_automatic_view = m_registry.view<Panel, InverseJointDirection, BoxJointPattern, FingerWidth, JointPatternDistance, AutomaticFingerPatternType>().proxy();
    for (auto &&[entity, panel, direction, pattern, finger_width, pattern_distance, pattern_type]: inverse_automatic_view) {
        PLOG_DEBUG << "Adding inverse joint finger patterns to " << panel.name;
        auto length = pattern_distance.value;
        auto width = finger_width.value;

        auto default_fingers = ceil(length / width);
        auto estimated_fingers = max(5.0, (floor(default_fingers / 2) * 2) - 1);
        auto actual_finger_width = length / estimated_fingers;
        auto pattern_offset = actual_finger_width * 2;
        auto actual_number_fingers = (ceil(estimated_fingers / 2) - 2);
        auto distance = (actual_number_fingers - 1) * 2 * actual_finger_width;
        auto corner_width = length / estimated_fingers;
        auto corner_distance = length - corner_width;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width*2, distance, pattern_offset, corner_width, corner_distance
        );
    }

    auto normal_constant_view = m_registry.view<FingerWidth, JointPatternDistance, ConstantFingerPatternType, BoxJointPattern>().proxy();
    for (auto &&[entity, finger_width, pattern_distance, pattern_type, joint_type]: normal_constant_view) {
        auto user_finger_width = finger_width.value;
        auto panel_length = pattern_distance.value - user_finger_width*2; // Make sure that we don't end up with a short finger on the ends

        auto default_fingers = ceil(panel_length / user_finger_width);
        auto estimated_fingers = (floor(default_fingers / 2) * 2) - 1;
        auto actual_finger_width = user_finger_width;

        auto actual_number_fingers = estimated_fingers < 3 ? 1 : ceil(estimated_fingers / 2);

        auto distance = estimated_fingers < 3 ? 0 : (estimated_fingers - 1) * actual_finger_width;
        auto pattern_multiplier = estimated_fingers < 3 ? user_finger_width : (estimated_fingers * actual_finger_width);
        auto pattern_offset = (pattern_distance.value - pattern_multiplier) / 2;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width*2, distance, pattern_offset
        );
    }

    auto inverse_constant_view = m_registry.view<JointPatternValues, InverseJointDirection, JointPatternDistance, ConstantFingerPatternType, BoxJointPattern>().proxy();
    for (auto &&[entity, values, pattern, distance, pattern_type, joint_type]: inverse_constant_view) {
        auto finger_count = values.finger_count + 1;
        auto corner_offset = values.pattern_offset;

        if (finger_count <= 2) {
            values.finger_count = 0;
            values.pattern_distance = 0;
        }

        values.finger_count =  finger_count - 2;
        values.finger_offset = values.finger_width*2;
        values.pattern_distance -= values.finger_width * 2;
        values.pattern_offset += values.finger_width;
        values.corner_width = corner_offset;
        values.corner_distance = distance.value - values.corner_width;
    }

    auto tenon_automatic_view = m_registry.view<TenonJointPattern, JointPatternDistance, FingerWidth, NormalJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: tenon_automatic_view) {
        auto shoulder = finger_width.value/2;
        auto actual_finger_width = pattern_distance.value - shoulder*2;
        auto pattern_offset = 0.0 + shoulder;
        auto actual_number_fingers = 1;
        auto distance = pattern_distance.value;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width*2, distance, pattern_offset
        );
    }

    auto mortise_automatic_view = m_registry.view<const TenonJointPattern, const JointPatternDistance, const FingerWidth, const InverseJointDirection>().proxy();
    for (auto &&[entity, pattern, distance, finger_width, direction]: mortise_automatic_view) {
        auto corner_width = finger_width.value/2;
        auto corner_distance = distance.value - corner_width;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, 0, 0.0, 0.0, 0.0, 0.0, corner_width, corner_distance
        );
    }

    auto double_tenon_automatic_view = m_registry.view<const DoubleTenonJointPattern, const JointPatternDistance, const FingerWidth, const NormalJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: double_tenon_automatic_view) {
        auto actual_number_fingers = 2;
        auto shoulder = finger_width.value/2;
        auto tenon_width = (pattern_distance.value - finger_width.value - shoulder)/2;
        auto pattern_offset = shoulder;
        auto distance = (tenon_width*actual_number_fingers) + shoulder - tenon_width;
        auto finger_offset = shoulder + tenon_width;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, actual_number_fingers, tenon_width, finger_offset, distance, pattern_offset
        );
    }

    auto double_mortise_automatic_view = m_registry.view<const DoubleTenonJointPattern, const JointPatternDistance, const FingerWidth, const InverseJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: double_mortise_automatic_view) {
        auto shoulder = finger_width.value/2;
        auto corner_distance = pattern_distance.value - shoulder;
        auto mortise_width = (pattern_distance.value - finger_width.value - shoulder)/2;
        auto pattern_offset = shoulder + mortise_width;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, 1, shoulder, 0.0, 0.0, pattern_offset, shoulder, corner_distance
        );
    }

    auto triple_tenon_automatic_view = m_registry.view<const TripleTenonJointPattern, const JointPatternDistance, const FingerWidth, const NormalJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: triple_tenon_automatic_view) {
        auto actual_number_fingers = 3;
        auto shoulder = finger_width.value/2;
        auto tenon_width = (pattern_distance.value - finger_width.value - (shoulder*(actual_number_fingers-1)))/actual_number_fingers;
        auto pattern_offset = shoulder;
        auto distance = tenon_width*(actual_number_fingers-1) + shoulder*(actual_number_fingers-1);
        auto finger_offset = shoulder + tenon_width;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, actual_number_fingers, tenon_width, finger_offset, distance, pattern_offset
        );
    }

    auto triple_mortise_automatic_view = m_registry.view<const TripleTenonJointPattern, const JointPatternDistance, const FingerWidth, const InverseJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: triple_mortise_automatic_view) {
        auto actual_number_fingers = 3;
        auto shoulder = finger_width.value/2;
        auto corner_distance = pattern_distance.value - shoulder;
        auto mortise_width = (pattern_distance.value - finger_width.value - (shoulder*(actual_number_fingers-1)))/actual_number_fingers;
        auto pattern_offset = shoulder + mortise_width;
        auto distance = mortise_width + shoulder;
        auto finger_offset = distance;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, actual_number_fingers - 1, shoulder, finger_offset, distance, pattern_offset, shoulder, corner_distance
        );
    }

    auto quad_tenon_automatic_view = m_registry.view<const QuadTenonJointPattern, const JointPatternDistance, const FingerWidth, const NormalJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: quad_tenon_automatic_view) {
        auto actual_number_fingers = 4;
        auto shoulder = finger_width.value/2;
        auto tenon_width = (pattern_distance.value - finger_width.value - (shoulder*(actual_number_fingers-1)))/actual_number_fingers;
        auto pattern_offset = shoulder;
        auto distance = tenon_width*(actual_number_fingers-1) + shoulder*(actual_number_fingers-1);
        auto finger_offset = shoulder + tenon_width;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, actual_number_fingers, tenon_width, finger_offset, distance, pattern_offset
        );
    }

    auto quad_mortise_automatic_view = m_registry.view<const QuadTenonJointPattern, const JointPatternDistance, const FingerWidth, const InverseJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: quad_mortise_automatic_view) {
        auto actual_number_fingers = 4;
        auto shoulder = finger_width.value/2;
        auto corner_distance = pattern_distance.value - shoulder;
        auto mortise_width = (pattern_distance.value - finger_width.value - (shoulder*(actual_number_fingers-1)))/actual_number_fingers;
        auto pattern_offset = shoulder + mortise_width;
        auto distance = mortise_width*2 + shoulder*2;
        auto finger_offset = pattern_offset;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, actual_number_fingers - 1, shoulder, finger_offset, distance, pattern_offset, shoulder, corner_distance
        );
    }

    auto toplap_automatic_view = m_registry.view<const LapJointPattern, const JointPatternDistance, const NormalJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, direction]: toplap_automatic_view) {
        auto actual_finger_width = pattern_distance.value / 2;
        auto pattern_offset = 0.0;
        auto actual_number_fingers = 1;
        auto distance = pattern_distance.value - actual_finger_width;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    }

    auto bottomlap_automatic_view = m_registry.view<const LapJointPattern, const JointPatternDistance, const InverseJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, direction]: bottomlap_automatic_view) {
        auto actual_finger_width = pattern_distance.value / 2;
        auto pattern_offset = 0.0 + actual_finger_width;
        auto actual_number_fingers = 1;
        auto distance = pattern_distance.value - actual_finger_width;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    }

    auto trim_automatic_view = m_registry.view<const TrimJointPattern, const FingerWidth, const JointPatternDistance, const NormalJointDirection>().proxy();
    for (auto &&[entity, pattern, finger_width, pattern_distance, direction]: trim_automatic_view) {
        auto pattern_length = pattern_distance.value;

        auto actual_finger_width = pattern_length;
        auto pattern_offset = 0.0;
        auto actual_number_fingers = 1;
        auto distance = pattern_length;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    }

    auto kerf_view = m_registry.view<JointPatternValues, const Kerf>().proxy();
    for (auto &&[entity, values, kerf]: kerf_view) {
        values.finger_width -= kerf.value;
        values.pattern_offset += kerf.value;
    }
    PLOG_DEBUG << "Finished addFingerParameters";
}

void ConfigureJoints::updateJointProfiles() {
    PLOG_DEBUG << "Started updateJointProfiles";
    auto outside_view = m_registry.view<JointProfile, const JointPatternValues>().proxy();
    for (auto &&[entity, group, values]: outside_view) {
        PLOG_DEBUG << "Updating box joint profile";
        group.finger_width = values.finger_width;
        group.finger_count = values.finger_count;
        group.pattern_distance = values.pattern_distance;
        group.pattern_offset = values.pattern_offset;
        group.finger_offset = values.finger_offset;
    }

    auto corner_kerf_view = m_registry.view<JointProfile, const InverseJointDirection, const Kerf>().proxy();
    for (auto &&[entity, group, pattern, kerf]: corner_kerf_view) {
        if (group.corner_width == 0) return;
        group.pattern_distance += kerf.value;
        group.pattern_offset -= kerf.value;
        group.finger_width += kerf.value;
    }

    auto toplap_kerf_view = m_registry.view<JointProfile, const LapJointPattern, const Kerf, const NormalJointDirection>().proxy();
    for (auto &&[entity, group, pattern, kerf, direction]: toplap_kerf_view) {
        group.finger_width += kerf.value;
    }

    auto bottomlap_kerf_view = m_registry.view<JointProfile, const LapJointPattern, const Kerf, const InverseJointDirection>().proxy();
    for (auto &&[entity, group, pattern, kerf, direction]: bottomlap_kerf_view) {
        group.finger_width += kerf.value;
        group.pattern_offset += kerf.value;
    }

    auto trim_kerf_view = m_registry.view<JointProfile, const TrimJointPattern, const Kerf>().proxy();
    for (auto &&[entity, group, pattern, kerf]: trim_kerf_view) {
        group.finger_width += kerf.value * 1.5;
    }

    auto inverse_view = m_registry.view<JointProfile,const JointPatternValues, const InverseJointDirection>().proxy();
    for (auto &&[entity, group, pattern, direction]: inverse_view) {
       group.corner_width = pattern.corner_width;
       group.corner_distance = pattern.corner_distance;
    }
    PLOG_DEBUG << "Finished updateJointProfiles";
}

void ConfigureJoints::updateJointProfileGroups() const {
    PLOG_DEBUG << "Started updateJointProfileGroups";
    auto hashes = std::map<entt::entity, std::set<size_t>>{};

    auto profile_view = m_registry.view<const JointProfile, const ParentPanel>().proxy();
    for (auto &&[entity, profile, parent]: profile_view) {
        PLOG_DEBUG << "Storing joint profile from " << (int)entity << " to " << (int)parent.id;
        auto hash = std::hash<JointProfile>()(profile);
        hashes[parent.id].insert(hash);
        PLOG_DEBUG << "hash is " << hash;
    }

    auto children_view = m_registry.view<const ChildPanels>().proxy();
    for (auto &&[entity, children]: children_view) {
        for (auto const& child: children.panels) {
            PLOG_DEBUG << "Updating " << (int)child << " with joint profile group from " << (int)entity;
            m_registry.emplace<JointProfileGroup>(child, hashes[entity]);
        }
    }
    PLOG_DEBUG << "Finished updateJointProfileGroups";
}

void ConfigureJoints::addJointGroups() {
    PLOG_DEBUG << "Started addJointGroups";
    auto joint_group_view = m_registry.view<const JointThickness, const JointProfile, const JointPatternPosition>().proxy();
    for (auto &&[entity, thickness, profile, pattern]: joint_group_view) {
        PLOG_DEBUG << "adding Joint Group";
        m_registry.emplace_or_replace<JointGroup>(
            entity, profile, thickness, pattern.panel_position, pattern.joint_position
        );
    }
    PLOG_DEBUG << "Finished addJointGroups";
}
