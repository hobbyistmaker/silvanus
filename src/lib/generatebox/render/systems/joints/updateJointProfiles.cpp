//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/ChildPanels.hpp"
#include "entities/JointDirection.hpp"
#include "entities/JointPatternTags.hpp"
#include "entities/JointPatternValue.hpp"
#include "entities/JointProfile.hpp"
#include "entities/JointGroupTag.hpp"
#include "entities/Kerf.hpp"
#include "entities/ParentPanel.hpp"

#include <entt/entt.hpp>
#include <plog/Log.h>
#include <fmt/format.h>

using std::max;

using namespace silvanus::generatebox::entities;

void updateJointProfileGroups(entt::registry& registry) {
    PLOG_DEBUG << "Started updateJointProfileGroups";
    auto hashes = std::map<entt::entity, std::set<size_t>>{};

    auto profile_view = registry.view<const JointProfile, const ParentPanel>().proxy();
    for (auto &&[entity, profile, parent]: profile_view) {
        PLOG_DEBUG << "Storing joint profile from " << (int)entity << " to " << (int)parent.id;
        auto hash = std::hash<JointProfile>()(profile);
        hashes[parent.id].insert(hash);
        PLOG_DEBUG << "hash is " << hash;
    }

    auto children_view = registry.view<const ChildPanels>().proxy();
    for (auto &&[entity, children]: children_view) {
        for (auto const& child: children.panels) {
            PLOG_DEBUG << "Updating " << (int)child << " with joint profile group from " << (int)entity;
            registry.emplace<JointGroupTag>(child, hashes[entity]);
        }
    }
    PLOG_DEBUG << "Finished updateJointProfileGroups";
}

void updateJointProfileExpressions(entt::registry &registry) {
    PLOG_DEBUG << "Started updateJointProfileExpressions";
    auto outside_view = registry.view<JointProfileParams, const JointPatternExpressions>().proxy();
    for (auto &&[entity, profile, values]: outside_view) {
        PLOG_DEBUG << "Updating box joint profile";
        profile.finger_width = values.finger_width;
        profile.finger_count = values.finger_count;
        profile.pattern_distance = values.pattern_distance;
        profile.pattern_offset = values.pattern_offset;
        profile.finger_offset = values.finger_offset;
    }

    auto toplap_kerf_view = registry.view<JointProfileParams, const LapJointPattern, const KerfParam, const NormalJointDirection>().proxy();
    for (auto &&[entity, profile, pattern, kerf, direction]: toplap_kerf_view) {
        auto kerf_expr = fmt::format(" + {0}", kerf.expression);
        profile.finger_width.shrink_to_fit();
        auto has_width = profile.finger_width.length() > 0;

        auto width_kerf_expr = has_width ? kerf_expr : "";

        profile.finger_width = has_width ? fmt::format("{0}{1}", profile.finger_width, width_kerf_expr) : "";
    }

    auto bottomlap_kerf_view = registry.view<JointProfileParams, const LapJointPattern, const KerfParam, const InverseJointDirection>().proxy();
    for (auto &&[entity, profile, pattern, kerf, direction]: bottomlap_kerf_view) {
        auto kerf_expr = fmt::format(" + {0}", kerf.expression);
        profile.finger_width.shrink_to_fit();
        auto has_width = profile.finger_width.length() > 0;

        auto width_kerf_expr = has_width ? kerf_expr : "";

        profile.finger_width = has_width ? fmt::format("{0}{1}", profile.finger_width, width_kerf_expr) : "";
        profile.pattern_offset = "";
    }

    auto trim_kerf_view = registry.view<JointProfileParams, const TrimJointPattern, const KerfParam>().proxy();
    for (auto &&[entity, profile, pattern, kerf]: trim_kerf_view) {
        auto kerf_expr = fmt::format(" + ({0} * 1.5)", kerf.expression);
        profile.finger_width.shrink_to_fit();
        auto has_width = profile.finger_width.length() > 0;

        auto width_kerf_expr = has_width ? kerf_expr : "";

        profile.finger_width = has_width ? fmt::format("{0}{1}", profile.finger_width, width_kerf_expr) : "";
    }

    auto inverse_view = registry.view<JointProfileParams, const JointPatternExpressions, const InverseJointDirection>().proxy();
    for (auto &&[entity, profile, pattern, direction]: inverse_view) {
        profile.corner_width = pattern.corner_width;
        profile.corner_distance = pattern.corner_distance;
    }
    PLOG_DEBUG << "Finished updateJointProfileExpressions";
}

void updateJointProfileValues(entt::registry &registry) {
    PLOG_DEBUG << "Started updateJointProfilesFromPatternValues";
    auto outside_view = registry.view<JointProfile, const JointPatternValues>();
    for (auto &&[entity, profile, values]: outside_view.proxy()) {
        PLOG_DEBUG << "Updating box joint profile";
        profile.finger_width = values.finger_width;
        profile.finger_count = values.finger_count;
        profile.pattern_distance = values.pattern_distance;
        profile.pattern_offset = values.pattern_offset;
        profile.finger_offset = values.finger_offset;
    }

    auto toplap_kerf_view = registry.view<JointProfile, const LapJointPattern, const Kerf, const NormalJointDirection>();
    for (auto &&[entity, profile, pattern, kerf, direction]: toplap_kerf_view.proxy()) {
        profile.finger_width += kerf.value;
    }

    auto bottomlap_kerf_view = registry.view<JointProfile, const LapJointPattern, const Kerf, const InverseJointDirection>();
    for (auto &&[entity, profile, pattern, kerf, direction]: bottomlap_kerf_view.proxy()) {
        profile.finger_width += kerf.value;
        profile.pattern_offset = 0;
    }

    auto trim_kerf_view = registry.view<JointProfile, const TrimJointPattern, const Kerf>();
    for (auto &&[entity, profile, pattern, kerf]: trim_kerf_view.proxy()) {
        profile.finger_width += kerf.value * 1.5;
    }

    auto inverse_view = registry.view<JointProfile, const JointPatternValues, const InverseJointDirection>();
    for (auto &&[entity, profile, pattern, direction]: inverse_view.proxy()) {
        profile.corner_width = pattern.corner_width;
        profile.corner_distance = pattern.corner_distance;
    }

    auto param_view = registry.view<JointProfile, const JointProfileParams>();
    for (auto &&[entity, profile, params]: param_view.proxy()) {
        profile.parameters = params;
    }
    PLOG_DEBUG << "Finished updateJointProfilesFromPatternValues";
}

void updateJointProfilesFromPatternValues(entt::registry& registry) {
    updateJointProfileExpressions(registry);
    updateJointProfileValues(registry);
}