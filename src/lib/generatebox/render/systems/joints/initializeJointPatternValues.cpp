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
#include <fmt/core.h>
#include <fmt/format.h> // Required for fmt/core to compile and link

using std::max;

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;

void initializeInverseTrimJointPatternValues(entt::registry &registry) {
    auto view = registry.view<const TrimJointPattern, const FingerWidth, const JointPatternDistance, const InverseJointDirection>();
    for (auto &&[entity, pattern, finger_width, pattern_distance, direction]: view.proxy()) {
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

void initializeInverseTrimJointPatternExpressions(entt::registry &registry) {
    auto view = registry.view<const TrimJointPattern, const FingerWidthParam, const JointPatternDistanceParam, const InverseJointDirection>();
    for (auto &&[entity, pattern, finger_width, pattern_distance, direction]: view.proxy()) {
        auto pattern_length = pattern_distance.expression;

        auto actual_finger_width = pattern_length;
        auto pattern_offset = "";
        auto actual_number_fingers = "1";
        auto distance = pattern_length;

        registry.emplace_or_replace<JointPatternExpressions>(
            entity, actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    }
}

void initializeNormalLapJointPatternValues(entt::registry &registry) {
    auto view = registry.view<const LapJointPattern, const JointPatternDistance, const NormalJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, direction]: view.proxy()) {
        auto actual_finger_width = pattern_distance.value / 2;
        auto pattern_offset = 0.0 + actual_finger_width;
        auto actual_number_fingers = 1;
        auto distance = pattern_distance.value - actual_finger_width;

        registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    }
}

void initializeNormalLapJointPatternExpressions(entt::registry &registry) {
    auto view = registry.view<const LapJointPattern, const JointPatternDistanceParam, const NormalJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, direction]: view.proxy()) {
        auto actual_finger_width = pattern_distance.expression + "/ 2";
        auto pattern_offset = actual_finger_width;
        auto actual_number_fingers = "1";
        auto distance = pattern_distance.expression + " - " + actual_finger_width;

        PLOG_DEBUG << "Lap joint pattern distance " << actual_finger_width;

        registry.emplace_or_replace<JointPatternExpressions>(
            entity, actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    }
}

void initializeInverseLapJointPatternValues(entt::registry &registry) {
    auto view = registry.view<const LapJointPattern, const JointPatternDistance, const InverseJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, direction]: view.proxy()) {
        auto actual_finger_width = pattern_distance.value / 2;
        auto pattern_offset = 0.0;
        auto actual_number_fingers = 1;
        auto distance = pattern_distance.value - actual_finger_width;

        registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    }
}

void initializeInverseLapJointPatternExpressions(entt::registry &registry) {
    auto view = registry.view<const LapJointPattern, const JointPatternDistanceParam, const InverseJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, direction]: view.proxy()) {
        auto actual_finger_width = pattern_distance.expression + "/ 2";
        auto pattern_offset = "";
        auto actual_number_fingers = "1";
        auto distance = pattern_distance.expression + " - " + actual_finger_width;

        PLOG_DEBUG << "Lap joint pattern distance " << actual_finger_width;

        registry.emplace_or_replace<JointPatternExpressions>(
            entity, actual_number_fingers, actual_finger_width, actual_finger_width, distance, pattern_offset
        );
    }
}

void initializeInverseQuadTenonJointPatternValues(entt::registry &registry) {
    auto view = registry.view<const QuadTenonJointPattern, const JointPatternDistance, const FingerWidth, const InverseJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: view.proxy()) {
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

void initializeInverseQuadTenonJointPatternExpressions(entt::registry &registry) {
    auto view = registry.view<const QuadTenonJointPattern, const JointPatternDistanceParam, const FingerWidthParam, const InverseJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: view.proxy()) {
        auto divisor = 4;
        auto actual_number_fingers = std::to_string(divisor - 1);
        auto shoulder = fmt::format("({} / 2)", finger_width.expression);
        auto corner_distance = fmt::format("({} - {})", pattern_distance.expression, shoulder);
        auto mortise_width = fmt::format("(({} - {} - ({} * {})) / {})", pattern_distance.expression, finger_width.expression, shoulder, actual_number_fingers, divisor);
        auto pattern_offset = fmt::format("({} + {})", shoulder, mortise_width);
        auto distance = fmt::format("({} * 2 + {} * 2)", mortise_width, shoulder);
        auto finger_offset = pattern_offset;

        registry.emplace_or_replace<JointPatternExpressions>(
            entity, actual_number_fingers, shoulder, finger_offset, distance, pattern_offset, shoulder, corner_distance
        );
    }
}

void initializeNormalQuadTenonJointPatternValues(entt::registry &registry) {
    auto view = registry.view<const QuadTenonJointPattern, const JointPatternDistance, const FingerWidth, const NormalJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: view.proxy()) {
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

void initializeNormalQuadTenonJointPatternExpressions(entt::registry &registry) {
    auto view = registry.view<const QuadTenonJointPattern, const JointPatternDistanceParam, const FingerWidthParam, const NormalJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: view.proxy()) {
        auto divisor = 4;
        auto actual_number_fingers = std::to_string(divisor - 1);
        auto shoulder = fmt::format("({} / 2)", finger_width.expression);
        auto shoulder_adjust = fmt::format("({} * {})", shoulder, actual_number_fingers);
        auto tenon_width = fmt::format("(({} - {} - {}) / {})", pattern_distance.expression, finger_width.expression, shoulder_adjust, divisor);
        auto pattern_offset = shoulder;
        auto distance = fmt::format("(({} * {}) + {} * {})", tenon_width, actual_number_fingers, shoulder, actual_number_fingers);
        auto finger_offset = fmt::format("({} + {})", shoulder, tenon_width);

        registry.emplace_or_replace<JointPatternExpressions>(
            entity, actual_number_fingers, tenon_width, finger_offset, distance, pattern_offset
        );
    }
}

void initializeInverseTripleTenonJointPatternValues(entt::registry &registry) {
    auto view = registry.view<const TripleTenonJointPattern, const JointPatternDistance, const FingerWidth, const InverseJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: view.proxy()) {
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

void initializeInverseTripleTenonJointPatternExpressions(entt::registry &registry) {
    auto view = registry.view<const TripleTenonJointPattern, const JointPatternDistanceParam, const FingerWidthParam, const InverseJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: view.proxy()) {
        auto divisor = 3;
        auto actual_number_fingers = std::to_string(divisor - 1);
        auto shoulder = fmt::format("({} / 2)", finger_width.expression);
        auto corner_distance = fmt::format("({} - {})", pattern_distance.expression, shoulder);
        auto mortise_width = fmt::format("(({} - {} - ({}*{})) / {})", pattern_distance.expression, finger_width.expression, shoulder, actual_number_fingers, divisor);
        auto pattern_offset = fmt::format("({} + {})", shoulder, mortise_width);
        auto distance = fmt::format("({} + {})", mortise_width, shoulder);
        auto finger_offset = distance;

        registry.emplace_or_replace<JointPatternExpressions>(
            entity, actual_number_fingers, shoulder, finger_offset, distance, pattern_offset, shoulder, corner_distance
        );
    }
}

void initializeNormalTripleTenonJointPatternValues(entt::registry &registry) {
    auto view = registry.view<const TripleTenonJointPattern, const JointPatternDistance, const FingerWidth, const NormalJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: view.proxy()) {
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

void initializeNormalTripleTenonJointPatternExpressions(entt::registry &registry) {
    auto view = registry.view<const TripleTenonJointPattern, const JointPatternDistanceParam, const FingerWidthParam, const NormalJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: view.proxy()) {
        auto divisor = 3;
        auto actual_number_fingers = std::to_string(divisor - 1);
        auto shoulder = fmt::format("({} / 2)", finger_width.expression);
        auto shoulder_adjust = fmt::format("({} * {})", shoulder, actual_number_fingers);
        auto tenon_width = fmt::format("(({} - {} - {}) / {})", pattern_distance.expression, finger_width.expression, shoulder_adjust, divisor);
        auto pattern_offset = shoulder;
        auto distance = fmt::format("(({} * {}) + {} * {})", tenon_width, actual_number_fingers, shoulder, actual_number_fingers);
        auto finger_offset = fmt::format("({} + {})", shoulder, tenon_width);

        registry.emplace_or_replace<JointPatternExpressions>(
            entity, actual_number_fingers, tenon_width, finger_offset, distance, pattern_offset
        );
    }
}

void initializeInverseDoubleTenonJointPatternValues(entt::registry &registry) {
    auto view = registry.view<const DoubleTenonJointPattern, const JointPatternDistance, const FingerWidth, const InverseJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: view.proxy()) {
        auto shoulder = finger_width.value/2;
        auto corner_distance = pattern_distance.value - shoulder;
        auto mortise_width = (pattern_distance.value - finger_width.value - shoulder)/2;
        auto pattern_offset = shoulder + mortise_width;

        registry.emplace_or_replace<JointPatternValues>(
            entity, 1, shoulder, 0.0, 0.0, pattern_offset, shoulder, corner_distance
        );
    }
}

void initializeInverseDoubleTenonJointPatternExpressions(entt::registry &registry) {
    auto view = registry.view<const DoubleTenonJointPattern, const JointPatternDistanceParam, const FingerWidthParam, const InverseJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: view.proxy()) {
        auto shoulder = fmt::format("({} / 2)", finger_width.expression);
        auto corner_distance = fmt::format("({} - {})", pattern_distance.expression, shoulder);
        auto mortise_width = fmt::format("(({} - {} - {}) / 2)", pattern_distance.expression, finger_width.expression, shoulder);
        auto pattern_offset = fmt::format("({} + {})", shoulder, mortise_width);

        registry.emplace_or_replace<JointPatternExpressions>(
            entity, "1", shoulder, "", "", pattern_offset, shoulder, corner_distance
        );
    }
}

void initializeNormalDoubleTenonJointPatternValues(entt::registry &registry) {
    auto view = registry.view<const DoubleTenonJointPattern, const JointPatternDistance, const FingerWidth, const NormalJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: view.proxy()) {
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

void initializeNormalDoubleTenonJointPatternExpressions(entt::registry &registry) {
    auto view = registry.view<const DoubleTenonJointPattern, const JointPatternDistanceParam, const FingerWidthParam, const NormalJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: view.proxy()) {
        auto divisor = 2;
        auto actual_number_fingers = std::to_string(divisor);
        auto shoulder = fmt::format("({} / 2)", finger_width.expression);
        auto tenon_width = fmt::format("(({} - {} - {}) / {})", pattern_distance.expression, finger_width.expression, shoulder, actual_number_fingers);
        auto pattern_offset = shoulder;
        auto distance = fmt::format("({} * {}) + {} - {}", tenon_width, actual_number_fingers, shoulder, tenon_width);
        auto finger_offset = fmt::format("({} + {})", shoulder, tenon_width);

        registry.emplace_or_replace<JointPatternExpressions>(
            entity, actual_number_fingers, tenon_width, finger_offset, distance, pattern_offset
        );
    }
}

void initializeInverseTenonJointPatternExpressions(entt::registry &registry) {
    auto view = registry.view<const TenonJointPattern, const JointPatternDistanceParam, const FingerWidthParam, const InverseJointDirection>();
    for (auto &&[entity, pattern, distance, finger_width, direction]: view.proxy()) {
        auto corner_width = fmt::format("({} / 2)", finger_width.expression);
        auto corner_distance = fmt::format("({} - {})", distance.expression, corner_width);

        registry.emplace_or_replace<JointPatternExpressions>(
            entity, "", "", "", "", "", corner_width, corner_distance
        );
    }
}

void initializeInverseTenonJointPatternValues(entt::registry &registry) {
    auto view = registry.view<const TenonJointPattern, const JointPatternDistance, const FingerWidth, const InverseJointDirection>();
    for (auto &&[entity, pattern, distance, finger_width, direction]: view.proxy()) {
        auto corner_width = finger_width.value/2;
        auto corner_distance = distance.value - corner_width;

        registry.emplace_or_replace<JointPatternValues>(
            entity, 0, 0.0, 0.0, 0.0, 0.0, corner_width, corner_distance
        );
    }
}

void initializeNormalTenonJointPatternValues(entt::registry &registry) {
    auto view = registry.view<const TenonJointPattern, const JointPatternDistance, const FingerWidth, const NormalJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: view.proxy()) {
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

void initializeNormalTenonJointPatternExpressions(entt::registry &registry) {
    auto view = registry.view<const TenonJointPattern, const JointPatternDistanceParam, const FingerWidthParam, const NormalJointDirection>();
    for (auto &&[entity, pattern, pattern_distance, finger_width, direction]: view.proxy()) {
        auto shoulder = finger_width.expression + "/2";
        auto actual_finger_width = pattern_distance.expression + shoulder + "* 2";
        auto pattern_offset = shoulder;
        auto actual_number_fingers = "1";
        auto distance = pattern_distance.expression;
        auto finger_offset = actual_finger_width + "* 2";

        registry.emplace_or_replace<JointPatternExpressions>(
            entity, actual_number_fingers, actual_finger_width, finger_offset, distance, pattern_offset
        );
    }
}

void updateInverseConstantWidthBoxJointPatternValues(entt::registry &registry) {
    auto view = registry.view<JointPatternValues, InverseJointDirection, JointPatternDistance, ConstantFingerPatternType, BoxJointPattern>();
    for (auto &&[entity, values, pattern, distance, pattern_type, joint_type]: view.proxy()) {
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

void updateInverseConstantWidthBoxJointPatternExpressions(entt::registry &registry) {
    auto view = registry.view<JointPatternExpressions, const JointPatternValues, const InverseJointDirection, const JointPatternDistanceParam, const ConstantFingerPatternType, const BoxJointPattern>();
    for (auto &&[entity, expressions, values, pattern, distance, pattern_type, joint_type]: view.proxy()) {
        auto corner_offset = std::string{"("}.append(expressions.pattern_offset).append(")");

        if (values.finger_count == 0) {
            expressions.finger_count = "";
            expressions.pattern_distance = "";
        }

        expressions.finger_count = std::string{"("}.append(expressions.finger_count).append("- 1)");
        expressions.finger_offset = std::string{"("}.append(expressions.finger_width).append("* 2)");
        expressions.pattern_distance = expressions.pattern_distance + std::string{"- ("}.append(expressions.finger_width).append("* 2)");
        expressions.pattern_offset = std::string{"("}.append(expressions.pattern_offset).append(" + ").append(expressions.finger_width).append(")");
        expressions.corner_width = corner_offset;
        expressions.corner_distance = std::string{"("}.append(distance.expression).append(" - ").append(expressions.corner_width).append(")");

        PLOG_DEBUG << "Updating inverse constant width joint pattern expressions";
        PLOG_DEBUG << "Finger count: " << expressions.finger_count;
        PLOG_DEBUG << "Finger offset: " << expressions.finger_offset;
        PLOG_DEBUG << "Pattern distance: " << expressions.pattern_distance;
        PLOG_DEBUG << "Pattern offset: " << expressions.pattern_offset;
        PLOG_DEBUG << "Adjusting corner width to " << expressions.corner_width << " and corner distance to " << expressions.corner_distance;
    }
}

void initializeAllConstantWidthBoxJointPatternValues(entt::registry &registry) {
    auto view = registry.view<const FingerWidth, const JointPatternDistance, const ConstantFingerPatternType, const BoxJointPattern>();
    for (auto &&[entity, finger_width, pattern_distance, pattern_type, joint_type]: view.proxy()) {
        auto user_finger_width = finger_width.value;
        auto panel_length = pattern_distance.value - user_finger_width*2; // Make sure that we don't end up with a short finger on the ends

        auto default_fingers = ceil(panel_length / user_finger_width);
        auto estimated_fingers = (floor(default_fingers / 2) * 2) - 1;
        auto actual_finger_width = user_finger_width;

        auto actual_number_fingers = estimated_fingers < 3 ? 1 : ceil(estimated_fingers / 2);

        auto distance = estimated_fingers < 3 ? 0 : (estimated_fingers - 1) * actual_finger_width;
        auto pattern_multiplier = estimated_fingers < 3 ? user_finger_width : (estimated_fingers * actual_finger_width);
        auto pattern_offset = (pattern_distance.value - pattern_multiplier) / 2;
        auto finger_offset = actual_finger_width * 2;

        registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, finger_offset, distance, pattern_offset
        );
    }
}

void initializeAllConstantWidthBoxJointPatternExpressions(entt::registry &registry) {
    auto view = registry.view<const FingerWidthParam, const JointPatternDistanceParam, const ConstantFingerPatternType, const BoxJointPattern>();
    for (auto &&[entity, finger_width, pattern_distance, pattern_type, joint_type]: view.proxy()) {
        auto user_finger_width = finger_width.expression;
        auto panel_length = std::string{pattern_distance.expression}.append(" - ").append(user_finger_width).append("*2"); // Make sure that we don't end up with a short finger on the ends

        auto default_fingers = std::string{"ceil(("}.append(panel_length).append(")/").append(user_finger_width).append(")");
        auto estimated_fingers = std::string{"((floor("}.append(default_fingers).append("/ 2) * 2) - 1)");
        auto actual_finger_width = std::string{user_finger_width};

        auto actual_number_fingers = std::string{"ceil(max(3; "}.append(estimated_fingers).append(")/2)");

        auto distance = std::string{"max(0; "}.append(estimated_fingers).append("- 1) *").append(actual_finger_width);
        auto pattern_multiplier = std::string{"("}.append(estimated_fingers).append("*").append(actual_finger_width).append(")");
        auto pattern_offset = std::string{"("}.append(pattern_distance.expression).append(" - ").append(pattern_multiplier).append(") / 2");
        auto finger_offset = std::string{"("}.append(actual_number_fingers).append("*2");

        registry.emplace_or_replace<JointPatternExpressions>(
            entity, actual_number_fingers, actual_finger_width, finger_offset, distance, pattern_offset
        );
    }
}

void initializeInverseAutomaticWidthBoxJointPatternValues(entt::registry &registry) {
    auto view = registry.view<const Panel, const InverseJointDirection, const BoxJointPattern, const FingerWidth, const JointPatternDistance, const AutomaticFingerPatternType>();
    for (auto &&[entity, panel, direction, pattern, finger_width, pattern_distance, pattern_type]: view.proxy()) {
        PLOG_DEBUG << "Adding inverse joint finger patterns to " << panel.name;
        auto length = pattern_distance.value;
        auto width = finger_width.value;

        auto default_fingers = ceil(length / width);
        auto estimated_fingers = max(5.0, (floor(default_fingers / 2) * 2) - 1);
        auto actual_finger_width = length / estimated_fingers;
        auto pattern_offset = actual_finger_width * 2;
        auto actual_number_fingers = (ceil(estimated_fingers / 2) - 2);
        auto distance = (actual_number_fingers - 1) * 2 * actual_finger_width;
        auto finger_offset = actual_finger_width * 2;
        auto corner_width = actual_finger_width;
        auto corner_distance = length - corner_width;

        PLOG_DEBUG << "Adjusting corner width to " << corner_width << " and corner distance to " << corner_distance;

        registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, finger_offset, distance, pattern_offset, corner_width, corner_distance
        );
    }
}

void initializeInverseAutomaticWidthBoxJointPatternExpressions(entt::registry &registry) {
    auto view = registry.view<const Panel, const InverseJointDirection, const BoxJointPattern, const FingerWidthParam, const JointPatternDistanceParam, const AutomaticFingerPatternType>();
    for (auto &&[entity, panel, direction, pattern, finger_width, pattern_distance, pattern_type]: view.proxy()) {
        PLOG_DEBUG << "Adding inverse joint finger patterns to " << panel.name;
        auto length = pattern_distance.expression;
        auto width = finger_width.expression;

        auto default_fingers = std::string{"ceil(("}.append(length).append(") / ( ").append(width).append("))");
        auto estimated_fingers = std::string{"max(5.0; (floor(("}.append(default_fingers).append(")/2)*2)-1)");
        auto actual_finger_width = std::string{"("}.append(length).append("/").append(estimated_fingers).append(")");
        auto pattern_offset = std::string{"("}.append(actual_finger_width).append("* 2)");
        auto actual_number_fingers = std::string{"(ceil("}.append(estimated_fingers).append("/ 2) - 2)");
        auto distance = std::string{"(("}.append(actual_number_fingers).append("- 1) * 2 *").append(actual_finger_width).append(")");
        auto corner_width = actual_finger_width;
        auto corner_distance = std::string{"("}.append(length).append("-").append(corner_width).append(")");
        auto finger_offset = std::string{"("}.append(actual_finger_width).append("* 2)");

        PLOG_DEBUG << "Adjusting corner width to " << corner_width << " and corner distance to " << corner_distance;

        registry.emplace_or_replace<JointPatternExpressions>(
            entity, actual_number_fingers, actual_finger_width, finger_offset, distance, pattern_offset, corner_width, corner_distance
        );
    }
}

void initializeNormalAutomaticWidthBoxJointPatternValues(entt::registry &registry) {
    auto view = registry.view<const Panel, const NormalJointDirection, const BoxJointPattern, const FingerWidth, const JointPatternDistance, const AutomaticFingerPatternType>();
    for (auto &&[entity, panel, direction, pattern, finger_width, pattern_distance, pattern_type]: view.proxy()) {
        PLOG_DEBUG << "Adding normal joint finger patterns to " << panel.name;
        auto length = pattern_distance.value;
        auto width = finger_width.value;

        auto default_fingers = ceil(length / width);
        auto estimated_fingers = max(5.0, (floor(default_fingers / 2) * 2) - 1);
        auto actual_finger_width = length / estimated_fingers;
        auto pattern_offset = actual_finger_width;
        auto actual_number_fingers = floor(estimated_fingers / 2);
        auto distance = (estimated_fingers - 3) * actual_finger_width;
        auto finger_offset = actual_finger_width * 2;

        registry.emplace_or_replace<JointPatternValues>(
            entity, (int)actual_number_fingers, actual_finger_width, finger_offset, distance, pattern_offset, 0.0, 0.0
        );
    }
}

void initializeNormalAutomaticWidthBoxJointPatternExpressions(entt::registry &registry) {
    auto view = registry.view<const Panel, const NormalJointDirection, const BoxJointPattern, const FingerWidthParam, const JointPatternDistanceParam, const AutomaticFingerPatternType>();
    for (auto &&[entity, panel, direction, pattern, finger_width, pattern_distance, pattern_type]: view.proxy()) {
        PLOG_DEBUG << "Adding normal joint finger pattern expressions to " << panel.name;
        auto length = pattern_distance.expression;
        auto width = finger_width.expression;

        auto default_fingers = fmt::format("ceil(({}) / ({}))", length, width);
        auto estimated_fingers = fmt::format("max(5.0; (floor(({}) / 2) * 2) - 1)", default_fingers);
        auto actual_finger_width = fmt::format("(({}) / ({}))", length, estimated_fingers);
        auto pattern_offset = actual_finger_width;
        auto actual_number_fingers = fmt::format("floor(({}) / 2)", estimated_fingers);
        auto distance = fmt::format("(({} - 3) * ({}))", estimated_fingers, actual_finger_width);
        auto finger_offset = fmt::format("({} * 2)", actual_finger_width);

        registry.emplace_or_replace<JointPatternExpressions>(
            entity, actual_number_fingers, actual_finger_width, finger_offset, distance, pattern_offset, "", ""
        );
    }
}
