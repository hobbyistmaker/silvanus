//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/FingerWidth.hpp"
#include "entities/FingerPattern.hpp"
#include "entities/JointDirection.hpp"
#include "entities/JointPatternDistance.hpp"
#include "entities/JointPatternTags.hpp"
#include "entities/JointPatternValue.hpp"
#include "entities/Panel.hpp"

#include <algorithm>
#include <unordered_map>

#include <entt/entt.hpp>
#include <plog/Log.h>

using std::max;

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;

void initializeInverseTrimJointPatternValues(entt::registry &registry) {
    auto trim_automatic_view = registry.view<const TrimJointPattern, const FingerWidth, const JointPatternDistance, const InverseJointDirection>().proxy();
    for (auto &&[entity, pattern, finger_width, pattern_distance, direction]: trim_automatic_view) {
        auto pattern_length = pattern_distance.value;

        auto actual_finger_width = pattern_length;
        auto pattern_offset = 0.0;
        auto actual_number_fingers = 1;
        auto distance = pattern_length;

        registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    }
}

void initializeNormalLapJointPatternValues(entt::registry &registry) {
    auto bottomlap_automatic_view = registry.view<const LapJointPattern, const JointPatternDistance, const NormalJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, direction]: bottomlap_automatic_view) {
        auto actual_finger_width = pattern_distance.value / 2;
        auto pattern_offset = 0.0 + actual_finger_width;
        auto actual_number_fingers = 1;
        auto distance = pattern_distance.value - actual_finger_width;

        registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    }
}

void initializeInverseLapJointPatternValues(entt::registry &registry) {
    auto toplap_automatic_view = registry.view<const LapJointPattern, const JointPatternDistance, const InverseJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, direction]: toplap_automatic_view) {
        auto actual_finger_width = pattern_distance.value / 2;
        auto pattern_offset = 0.0;
        auto actual_number_fingers = 1;
        auto distance = pattern_distance.value - actual_finger_width;

        registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    }
}

void initializeInverseQuadTenonJointPatternValues(entt::registry &registry) {
    auto quad_mortise_automatic_view = registry.view<const QuadTenonJointPattern, const JointPatternDistance, const FingerWidth, const InverseJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: quad_mortise_automatic_view) {
        auto actual_number_fingers = 4;
        auto shoulder = finger_width.value/2;
        auto corner_distance = pattern_distance.value - shoulder;
        auto mortise_width = (pattern_distance.value - finger_width.value - (shoulder*(actual_number_fingers-1)))/actual_number_fingers;
        auto pattern_offset = shoulder + mortise_width;
        auto distance = mortise_width*2 + shoulder*2;
        auto finger_offset = pattern_offset;

        registry.emplace_or_replace<JointPatternValues>(
            entity, actual_number_fingers - 1, shoulder, finger_offset, distance, pattern_offset, shoulder, corner_distance
        );
    }
}

void initializeNormalQuadTenonJointPatternValues(entt::registry &registry) {
    auto quad_tenon_automatic_view = registry.view<const QuadTenonJointPattern, const JointPatternDistance, const FingerWidth, const NormalJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: quad_tenon_automatic_view) {
        auto actual_number_fingers = 4;
        auto shoulder = finger_width.value/2;
        auto tenon_width = (pattern_distance.value - finger_width.value - (shoulder*(actual_number_fingers-1)))/actual_number_fingers;
        auto pattern_offset = shoulder;
        auto distance = tenon_width*(actual_number_fingers-1) + shoulder*(actual_number_fingers-1);
        auto finger_offset = shoulder + tenon_width;

        registry.emplace_or_replace<JointPatternValues>(
            entity, actual_number_fingers, tenon_width, finger_offset, distance, pattern_offset
        );
    }
}

void initializeInverseTripleTenonJointPatternValues(entt::registry &registry) {
    auto triple_mortise_automatic_view = registry.view<const TripleTenonJointPattern, const JointPatternDistance, const FingerWidth, const InverseJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: triple_mortise_automatic_view) {
        auto actual_number_fingers = 3;
        auto shoulder = finger_width.value/2;
        auto corner_distance = pattern_distance.value - shoulder;
        auto mortise_width = (pattern_distance.value - finger_width.value - (shoulder*(actual_number_fingers-1)))/actual_number_fingers;
        auto pattern_offset = shoulder + mortise_width;
        auto distance = mortise_width + shoulder;
        auto finger_offset = distance;

        registry.emplace_or_replace<JointPatternValues>(
            entity, actual_number_fingers - 1, shoulder, finger_offset, distance, pattern_offset, shoulder, corner_distance
        );
    }
}

void initializeNormalTripleTenonJointPatternValues(entt::registry &registry) {
    auto triple_tenon_automatic_view = registry.view<const TripleTenonJointPattern, const JointPatternDistance, const FingerWidth, const NormalJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: triple_tenon_automatic_view) {
        auto actual_number_fingers = 3;
        auto shoulder = finger_width.value/2;
        auto tenon_width = (pattern_distance.value - finger_width.value - (shoulder*(actual_number_fingers-1)))/actual_number_fingers;
        auto pattern_offset = shoulder;
        auto distance = tenon_width*(actual_number_fingers-1) + shoulder*(actual_number_fingers-1);
        auto finger_offset = shoulder + tenon_width;

        registry.emplace_or_replace<JointPatternValues>(
            entity, actual_number_fingers, tenon_width, finger_offset, distance, pattern_offset
        );
    }
}

void initializeInverseDoubleTenonJointPatternValues(entt::registry &registry) {
    auto double_mortise_automatic_view = registry.view<const DoubleTenonJointPattern, const JointPatternDistance, const FingerWidth, const InverseJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: double_mortise_automatic_view) {
        auto shoulder = finger_width.value/2;
        auto corner_distance = pattern_distance.value - shoulder;
        auto mortise_width = (pattern_distance.value - finger_width.value - shoulder)/2;
        auto pattern_offset = shoulder + mortise_width;

        registry.emplace_or_replace<JointPatternValues>(
            entity, 1, shoulder, 0.0, 0.0, pattern_offset, shoulder, corner_distance
        );
    }
}

void initializeNormalDoubleTenonJointPatternValues(entt::registry &registry) {
    auto double_tenon_automatic_view = registry.view<const DoubleTenonJointPattern, const JointPatternDistance, const FingerWidth, const NormalJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: double_tenon_automatic_view) {
        auto actual_number_fingers = 2;
        auto shoulder = finger_width.value/2;
        auto tenon_width = (pattern_distance.value - finger_width.value - shoulder)/2;
        auto pattern_offset = shoulder;
        auto distance = (tenon_width*actual_number_fingers) + shoulder - tenon_width;
        auto finger_offset = shoulder + tenon_width;

        registry.emplace_or_replace<JointPatternValues>(
            entity, actual_number_fingers, tenon_width, finger_offset, distance, pattern_offset
        );
    }
}

void initializeInverseTenonJointPatternValues(entt::registry &registry) {
    auto mortise_automatic_view = registry.view<const TenonJointPattern, const JointPatternDistance, const FingerWidth, const InverseJointDirection>().proxy();
    for (auto &&[entity, pattern, distance, finger_width, direction]: mortise_automatic_view) {
        auto corner_width = finger_width.value/2;
        auto corner_distance = distance.value - corner_width;

        registry.emplace_or_replace<JointPatternValues>(
            entity, 0, 0.0, 0.0, 0.0, 0.0, corner_width, corner_distance
        );
    }
}

void initializeNormalTenonJointPatternValues(entt::registry &registry) {
    auto tenon_automatic_view = registry.view<TenonJointPattern, JointPatternDistance, FingerWidth, NormalJointDirection>().proxy();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: tenon_automatic_view) {
        auto shoulder = finger_width.value/2;
        auto actual_finger_width = pattern_distance.value - shoulder*2;
        auto pattern_offset = 0.0 + shoulder;
        auto actual_number_fingers = 1;
        auto distance = pattern_distance.value;

        registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width*2, distance, pattern_offset
        );
    }
}

void updateInverseConstantWidthBoxJointPatternValues(entt::registry &registry) {
    auto inverse_constant_view = registry.view<JointPatternValues, InverseJointDirection, JointPatternDistance, ConstantFingerPatternType, BoxJointPattern>().proxy();
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

        PLOG_DEBUG << "Adjusting corner width to " << values.corner_width << " and corner distance to " << values.corner_distance;
    }
}

void initializeAllConstantWidthBoxJointPatternValues(entt::registry &registry) {
    auto normal_constant_view = registry.view<FingerWidth, JointPatternDistance, ConstantFingerPatternType, BoxJointPattern>().proxy();
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

        registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width*2, distance, pattern_offset
        );
    }
}

void initializeInverseAutomaticWidthBoxJointPatternValues(entt::registry &registry) {
    auto inverse_automatic_view = registry.view<Panel, InverseJointDirection, BoxJointPattern, FingerWidth, JointPatternDistance, AutomaticFingerPatternType>().proxy();
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
        auto corner_width = actual_finger_width;
        auto corner_distance = length - corner_width;

        PLOG_DEBUG << "Adjusting corner width to " << corner_width << " and corner distance to " << corner_distance;

        registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width*2, distance, pattern_offset, corner_width, corner_distance
        );
    }
}

void initializeNormalAutomaticWidthBoxJointPatternValues(entt::registry &registry) {
    auto normal_automatic_view = registry.view<Panel, NormalJointDirection, BoxJointPattern, FingerWidth, JointPatternDistance, AutomaticFingerPatternType>().proxy();
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

        registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width*2, distance, pattern_offset, 0.0, 0.0
        );
    }
}
