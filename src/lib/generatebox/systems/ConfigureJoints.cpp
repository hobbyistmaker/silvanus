//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/28/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "ConfigureJoints.hpp"
#include "entities/AxisFlag.hpp"
#include "entities/Dimensions.hpp"
#include "entities/Enabled.hpp"
#include "entities/EndReferencePoint.hpp"
#include "entities/FingerWidth.hpp"
#include "entities/FingerPatternType.hpp"
#include "entities/InsidePanel.hpp"
#include "entities/JoinedPanels.hpp"
#include "entities/JointGroup.hpp"
#include "entities/JointOrientation.hpp"
#include "entities/JointPatternDistance.hpp"
#include "entities/JointPatternPosition.hpp"
#include "entities/JointPatternTags.hpp"
#include "entities/JointPatternType.hpp"
#include "entities/JointPatternValue.hpp"
#include "entities/JointThickness.hpp"
#include "entities/Kerf.hpp"
#include "entities/OrientationGroup.hpp"
#include "entities/OutsidePanel.hpp"
#include "entities/PanelOrientation.hpp"
#include "entities/Position.hpp"

#include <algorithm>
#include <map>
#include <unordered_map>

using std::max;

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::systems;

void ConfigureJoints::execute() {
    updateJointPatternDistances();

    addFingerParameters();
    updateJointProfiles();
    addJointGroups();
    addJoints();
}

void ConfigureJoints::updateJointPatternDistances() {
    m_registry.view<JointPatternDistance, OrientationGroup, EndReferencePoint>().each([this](
        auto& pattern_distance, auto const& orientation, auto const& reference
    ) {
        auto const& distance = this->reference_selector[orientation.panel][orientation.finger](reference);
        pattern_distance.value = distance.value;
    });
}

void ConfigureJoints::addFingerParameters() {
    auto normal_automatic_view = m_registry.view<NormalJointPattern, FingerWidth, JointPatternDistance, AutomaticFingerPatternType>();
    normal_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& finger_width, auto const& pattern_distance, auto const& pattern_type
    ){
        auto length = pattern_distance.value;
        auto width = finger_width.value;

        auto default_fingers = ceil(length / width);
        auto estimated_fingers = max(5.0, (floor(default_fingers / 2) * 2) - 1);
        auto actual_finger_width = length / estimated_fingers;
        auto pattern_offset = actual_finger_width;
        auto actual_number_fingers = floor(estimated_fingers / 2);
        auto distance = (estimated_fingers - 3) * actual_finger_width;

        this->m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    });

    auto inverse_automatic_view = m_registry.view<InverseJointPattern, FingerWidth, JointPatternDistance, AutomaticFingerPatternType>();
    inverse_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& finger_width, auto const& pattern_distance, auto const& pattern_type
    ){
        auto length = pattern_distance.value;
        auto width = finger_width.value;

        auto default_fingers = ceil(length / width);
        auto estimated_fingers = max(5.0, (floor(default_fingers / 2) * 2) - 1);
        auto actual_finger_width = length / estimated_fingers;
        auto pattern_offset = actual_finger_width * 2;
        auto actual_number_fingers = (ceil(estimated_fingers / 2) - 2);
        auto distance = (actual_number_fingers - 1) * 2 * actual_finger_width;

        this->m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    });

    auto corner_automatic_view = m_registry.view<CornerJointPattern, FingerWidth, JointPatternDistance, AutomaticFingerPatternType>();
    corner_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& finger_width, auto const& pattern_distance, auto const& pattern_type
    ){
        auto pattern_length = pattern_distance.value;
        auto user_finger_width = finger_width.value;

        auto default_fingers = ceil(pattern_length / user_finger_width);
        auto estimated_fingers = max(5.0, (floor(default_fingers / 2) * 2) - 1);
        auto actual_finger_width = pattern_length / estimated_fingers;
        auto pattern_offset = 0.0;
        auto actual_number_fingers = 2;
        auto distance = pattern_length - actual_finger_width;

        this->m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    });

    auto normal_constant_view = m_registry.view<FingerWidth, JointPatternDistance, ConstantFingerPatternType>();
    normal_constant_view.each([this](
        auto entity, auto const& finger_width, auto const& pattern_distance, auto const& pattern_type
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

        this->m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    });

    auto inverse_constant_view = m_registry.view<JointPatternValues, InverseJointPattern, ConstantFingerPatternType>();
    inverse_constant_view.each([this](
        auto entity, auto& values, auto const& pattern, auto const& pattern_type
    ){
        auto finger_count = values.finger_count + 1;

        if (finger_count <= 2) {
            values.finger_count = 0;
            values.pattern_distance = 0;
            return;
        }

        values.finger_count =  finger_count - 2;
        values.finger_offset = values.finger_width;
        values.pattern_distance -= values.finger_width * 2;
        values.pattern_offset += values.finger_width;
    });

    auto corner_constant_view = m_registry.view<JointPatternValues, JointPatternDistance, CornerJointPattern, ConstantFingerPatternType>();
    corner_constant_view.each([this](
        auto entity, auto& values, auto const& distance, auto const& pattern, auto const& pattern_type
    ){
        values.finger_count = 2;
        values.finger_width = values.pattern_offset;
        values.pattern_distance = distance.value - values.finger_width;
        values.pattern_offset = 0;
    });


    auto tenon_automatic_view = m_registry.view<TenonJointPattern, JointPatternDistance>();
    tenon_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& pattern_distance
    ){
        auto actual_finger_width = pattern_distance.value / 3;
        auto pattern_offset = 0.0;
        auto actual_number_fingers = 2;
        auto distance = pattern_distance.value - actual_finger_width;

        this->m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    });

    auto mortise_automatic_view = m_registry.view<MortiseJointPattern, JointPatternDistance>();
    mortise_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& pattern_distance
    ){
        auto actual_finger_width = pattern_distance.value / 3;
        auto pattern_offset = 0.0 + actual_finger_width;
        auto actual_number_fingers = 1;
        auto distance = pattern_distance.value;

        this->m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    });

    auto toplap_automatic_view = m_registry.view<TopLapJointPattern, JointPatternDistance>();
    toplap_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& pattern_distance
    ){
        auto actual_finger_width = pattern_distance.value / 2;
        auto pattern_offset = 0.0;
        auto actual_number_fingers = 1;
        auto distance = pattern_distance.value - actual_finger_width;

        this->m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    });

    auto bottomlap_automatic_view = m_registry.view<BottomLapJointPattern, JointPatternDistance>();
    bottomlap_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& pattern_distance
    ){
        auto actual_finger_width = pattern_distance.value / 2;
        auto pattern_offset = 0.0 + actual_finger_width;
        auto actual_number_fingers = 1;
        auto distance = pattern_distance.value - actual_finger_width;

        this->m_registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    });

    auto trim_automatic_view = m_registry.view<TrimJointPattern, FingerWidth, JointPatternDistance>();
    trim_automatic_view.each([this](
        auto entity, auto const& pattern, auto const& finger_width, auto const& pattern_distance
    ){
        auto pattern_length = pattern_distance.value;

        auto actual_finger_width = pattern_length;
        auto pattern_offset = 0.0;
        auto actual_number_fingers = 1;
        auto distance = pattern_length;

        this->m_registry.emplace_or_replace<JointPatternValues>(
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
    auto inside_view = m_registry.view<JointProfile, JointPatternValues, JointThickness, InsidePanel>();
    inside_view.each([](
        auto& group, auto const& values, auto& thickness, auto const& panel
    ){
        group.finger_width = values.finger_width;
        group.finger_count = values.finger_count;
        group.pattern_distance = values.pattern_distance;
        group.pattern_offset = values.pattern_offset;
        group.finger_offset = values.finger_offset;
    });

    auto outside_view = m_registry.view<JointProfile, JointPatternValues, JointThickness, OutsidePanel>();
    outside_view.each([](
        auto& group, auto const& values, auto& thickness, auto const& panel
    ){
        group.finger_width = values.finger_width;
        group.finger_count = values.finger_count;
        group.pattern_distance = values.pattern_distance;
        group.pattern_offset = values.pattern_offset;
        group.finger_offset = values.finger_offset;
    });

    auto corner_kerf_view = m_registry.view<JointProfile, CornerJointPattern, Kerf>();
    corner_kerf_view.each([](
        auto& group, auto const& pattern, auto const& kerf
    ){
        group.pattern_distance += kerf.value;
        group.pattern_offset -= kerf.value;
        group.finger_width += kerf.value;
    });

    auto toplap_kerf_view = m_registry.view<JointProfile, TopLapJointPattern, Kerf>();
    toplap_kerf_view.each([](
        auto& group, auto const& pattern, auto const& kerf
    ){
        group.finger_width += kerf.value;
    });

    auto bottomlap_kerf_view = m_registry.view<JointProfile, BottomLapJointPattern, Kerf>();
    bottomlap_kerf_view.each([](
        auto& group, auto const& pattern, auto const& kerf
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
}

void ConfigureJoints::addJointGroups() {
    m_registry.view<JointThickness, JointProfile, JointPatternPosition>().each([this](
        auto entity, auto const& thickness, auto const& profile, auto const& pattern
    ){
        this->m_registry.emplace_or_replace<JointGroup>(
            entity, profile, thickness, pattern.panel_position, pattern.joint_position
        );
    });
}

void ConfigureJoints::addJoints() {
    std::map<Position, std::map<AxisFlag, std::map<Position, std::map<AxisFlag, std::vector<JointPanel>>>> > source_panels;

    m_registry.view<Enabled, JointPatternPosition, JointExtrusion, JointGroup>().each([&](
        auto& e, auto const& p, auto const& extrusion, auto const& group
    ){
        source_panels[p.joint_position][p.joint_orientation][p.panel_position][p.panel_orientation].emplace_back(JointPanel{group, extrusion});
    });

    m_registry.view<Enabled, JointPatternPosition>().each([&, this](
        auto entity, auto& enabled, auto const& pattern
    ){
        this->m_registry.emplace_or_replace<JoinedPanels>(
            entity, source_panels[pattern.panel_position][pattern.panel_orientation][pattern.joint_position][pattern.joint_orientation]
        );
    });
}
