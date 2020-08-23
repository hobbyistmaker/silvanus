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
#include "entities/FingerPatternType.hpp"
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
    m_registry.view<JointPatternDistance, OrientationGroup, EndReferencePoint>().each([this](
        auto& pattern_distance, auto const& orientation, auto const& reference
    ) {
        PLOG_DEBUG << "Updating Joint Pattern Distance: " << (int)orientation.finger << ":" << (int)orientation.panel;
        auto const& distance = reference_selector[orientation.panel][orientation.finger](reference);
        pattern_distance.value = distance.value;
    });
}

void ConfigureJoints::addFingerParameters() {
    auto normal_automatic_view = m_registry.view<Panel, NormalJointDirection, BoxJointPattern, FingerWidth, JointPatternDistance, AutomaticFingerPatternType>();
    normal_automatic_view.each([this](
        auto entity, auto const& panel, auto const& direction, auto const& pattern, auto const& finger_width, auto const& pattern_distance, auto const& pattern_type
    ){
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
    });

    auto inverse_automatic_view = m_registry.view<Panel, InverseJointDirection, BoxJointPattern, FingerWidth, JointPatternDistance, AutomaticFingerPatternType>();
    inverse_automatic_view.each([this](
        auto entity, auto const& panel, auto const& direction, auto const& pattern, auto const& finger_width, auto const& pattern_distance, auto const& pattern_type
    ){
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
    });

    auto normal_constant_view = m_registry.view<FingerWidth, JointPatternDistance, ConstantFingerPatternType, BoxJointPattern>();
    normal_constant_view.each([this](
        auto entity, auto const& finger_width, auto const& pattern_distance, auto const& pattern_type, auto const& joint_type
    ){
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
    });

    auto inverse_constant_view = m_registry.view<JointPatternValues, InverseJointDirection, JointPatternDistance, ConstantFingerPatternType, BoxJointPattern>();
    inverse_constant_view.each([](
        auto entity, auto& values, auto const& pattern, auto const& distance, auto const& pattern_type, auto const& joint_type
    ){
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
    });

    auto tenon_automatic_view = m_registry.view<TenonJointPattern, JointPatternDistance, FingerWidth, NormalJointDirection>();
    tenon_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& pattern_distance, auto const& finger_width, auto const& direction
    ){
        auto shoulder = finger_width.value/2;
        auto actual_finger_width = pattern_distance.value - shoulder*2;
        auto pattern_offset = 0.0 + shoulder;
        auto actual_number_fingers = 1;
        auto distance = pattern_distance.value;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width*2, distance, pattern_offset
        );
    });

    auto mortise_automatic_view = m_registry.view<TenonJointPattern, JointPatternDistance, FingerWidth, InverseJointDirection>();
    mortise_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& distance, auto const& finger_width, auto const& direction
    ){
        auto corner_width = finger_width.value/2;
        auto corner_distance = distance.value - corner_width;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, 0, 0.0, 0.0, 0.0, 0.0, corner_width, corner_distance
        );
    });

    auto double_tenon_automatic_view = m_registry.view<DoubleTenonJointPattern, JointPatternDistance, FingerWidth, NormalJointDirection>();
    double_tenon_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& pattern_distance, auto const& finger_width, auto const& direction
    ){
        auto actual_number_fingers = 2;
        auto shoulder = finger_width.value/2;
        auto tenon_width = (pattern_distance.value - finger_width.value - shoulder)/2;
        auto pattern_offset = shoulder;
        auto distance = (tenon_width*actual_number_fingers) + shoulder - tenon_width;
        auto finger_offset = shoulder + tenon_width;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, actual_number_fingers, tenon_width, finger_offset, distance, pattern_offset
        );
    });

    auto double_mortise_automatic_view = m_registry.view<DoubleTenonJointPattern, JointPatternDistance, FingerWidth, InverseJointDirection>();
    double_mortise_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& pattern_distance, auto const& finger_width, auto const& direction
    ){
        auto shoulder = finger_width.value/2;
        auto corner_distance = pattern_distance.value - shoulder;
        auto mortise_width = (pattern_distance.value - finger_width.value - shoulder)/2;
        auto pattern_offset = shoulder + mortise_width;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, 1, shoulder, 0.0, 0.0, pattern_offset, shoulder, corner_distance
        );
    });

    auto triple_tenon_automatic_view = m_registry.view<TripleTenonJointPattern, JointPatternDistance, FingerWidth, NormalJointDirection>();
    triple_tenon_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& pattern_distance, auto const& finger_width, auto const& direction
    ){
        auto actual_number_fingers = 3;
        auto shoulder = finger_width.value/2;
        auto tenon_width = (pattern_distance.value - finger_width.value - (shoulder*(actual_number_fingers-1)))/actual_number_fingers;
        auto pattern_offset = shoulder;
        auto distance = tenon_width*(actual_number_fingers-1) + shoulder*(actual_number_fingers-1);
        auto finger_offset = shoulder + tenon_width;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, actual_number_fingers, tenon_width, finger_offset, distance, pattern_offset
        );
    });

    auto triple_mortise_automatic_view = m_registry.view<TripleTenonJointPattern, JointPatternDistance, FingerWidth, InverseJointDirection>();
    triple_mortise_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& pattern_distance, auto const& finger_width, auto const& direction
    ){
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
    });

    auto quad_tenon_automatic_view = m_registry.view<QuadTenonJointPattern, JointPatternDistance, FingerWidth, NormalJointDirection>();
    quad_tenon_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& pattern_distance, auto const& finger_width, auto const& direction
    ){
        auto actual_number_fingers = 4;
        auto shoulder = finger_width.value/2;
        auto tenon_width = (pattern_distance.value - finger_width.value - (shoulder*(actual_number_fingers-1)))/actual_number_fingers;
        auto pattern_offset = shoulder;
        auto distance = tenon_width*(actual_number_fingers-1) + shoulder*(actual_number_fingers-1);
        auto finger_offset = shoulder + tenon_width;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, actual_number_fingers, tenon_width, finger_offset, distance, pattern_offset
        );
    });

    auto quad_mortise_automatic_view = m_registry.view<QuadTenonJointPattern, JointPatternDistance, FingerWidth, InverseJointDirection>();
    quad_mortise_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& pattern_distance, auto const& finger_width, auto const& direction
    ){
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
    });

    auto toplap_automatic_view = m_registry.view<LapJointPattern, JointPatternDistance, NormalJointDirection>();
    toplap_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& pattern_distance, auto const& direction
    ){
        auto actual_finger_width = pattern_distance.value / 2;
        auto pattern_offset = 0.0;
        auto actual_number_fingers = 1;
        auto distance = pattern_distance.value - actual_finger_width;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    });

    auto bottomlap_automatic_view = m_registry.view<LapJointPattern, JointPatternDistance, InverseJointDirection>();
    bottomlap_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& pattern_distance, auto const& direction
    ){
        auto actual_finger_width = pattern_distance.value / 2;
        auto pattern_offset = 0.0 + actual_finger_width;
        auto actual_number_fingers = 1;
        auto distance = pattern_distance.value - actual_finger_width;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    });

    auto trim_automatic_view = m_registry.view<TrimJointPattern, FingerWidth, JointPatternDistance, NormalJointDirection>();
    trim_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& finger_width, auto const& pattern_distance, auto const& direction
    ){
        auto pattern_length = pattern_distance.value;

        auto actual_finger_width = pattern_length;
        auto pattern_offset = 0.0;
        auto actual_number_fingers = 1;
        auto distance = pattern_length;

        m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    });

    auto kerf_view = m_registry.view<JointPatternValues, Kerf>();
    kerf_view.each([](
        auto& values, auto const& kerf
    ){
        values.finger_width -= kerf.value;
        values.pattern_offset += kerf.value;
    });
}

void ConfigureJoints::updateJointProfiles() {
    auto outside_view = m_registry.view<JointProfile, JointPatternValues>();
    outside_view.each([](
        auto& group, auto const& values
    ){
        PLOG_DEBUG << "Updating box joint profile";
        group.finger_width = values.finger_width;
        group.finger_count = values.finger_count;
        group.pattern_distance = values.pattern_distance;
        group.pattern_offset = values.pattern_offset;
        group.finger_offset = values.finger_offset;
    });

    auto corner_kerf_view = m_registry.view<JointProfile, InverseJointDirection, Kerf>();
    corner_kerf_view.each([](
        auto& group, auto const& pattern, auto const& kerf
    ){
        if (group.corner_width == 0) return;
        group.pattern_distance += kerf.value;
        group.pattern_offset -= kerf.value;
        group.finger_width += kerf.value;
    });

    auto toplap_kerf_view = m_registry.view<JointProfile, LapJointPattern, Kerf, NormalJointDirection>();
    toplap_kerf_view.each([](
        auto& group, auto const& pattern, auto const& kerf, auto const& direction
    ){
        group.finger_width += kerf.value;
    });

    auto bottomlap_kerf_view = m_registry.view<JointProfile, LapJointPattern, Kerf, InverseJointDirection>();
    bottomlap_kerf_view.each([](
        auto& group, auto const& pattern, auto const& kerf, auto const& direction
    ){
        group.finger_width += kerf.value;
        group.pattern_offset += kerf.value;
    });

    auto trim_kerf_view = m_registry.view<JointProfile, TrimJointPattern, Kerf>();
    trim_kerf_view.each([](
        auto& group, auto const& pattern, auto const& kerf
    ){
        group.finger_width += kerf.value * 1.5;
    });

    auto inverse_view = m_registry.view<JointProfile, JointPatternValues, InverseJointDirection>();
    inverse_view.each([](
        auto& group, auto const& pattern, auto const& direction
    ){
       group.corner_width = pattern.corner_width;
       group.corner_distance = pattern.corner_distance;
    });
}

void ConfigureJoints::updateJointProfileGroups() const {
    auto hashes = std::map<entt::entity, std::set<size_t>>{};

    m_registry.view<JointProfile, ParentPanel>().each([&](
        auto entity, JointProfile const& profile, ParentPanel const& parent
    ){
        PLOG_DEBUG << "Storing joint profile from " << (int)entity << " to " << (int)parent.id;
        auto hash = std::hash<JointProfile>()(profile);
        hashes[parent.id].insert(hash);
        PLOG_DEBUG << "hash is " << hash;
    });

    m_registry.view<ChildPanels>().each([&](
        auto entity, auto const& children
    ){
        for (auto const& child: children.panels) {
            PLOG_DEBUG << "Updating " << (int)child << " with joint profile group from " << (int)entity;
            m_registry.emplace<JointProfileGroup>(child, hashes[entity]);
        }
    });
}

void ConfigureJoints::addJointGroups() {
    m_registry.view<JointThickness, JointProfile, JointPatternPosition>().each([this](
        auto entity, auto const& thickness, auto const& profile, auto const& pattern
    ){
        PLOG_DEBUG << "adding Joint Group";
        m_registry.emplace_or_replace<JointGroup>(
            entity, profile, thickness, pattern.panel_position, pattern.joint_position
        );
    });
}
