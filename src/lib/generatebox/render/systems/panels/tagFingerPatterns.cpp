//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/FingerPattern.hpp"
#include "entities/JointProfile.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void tagNoFingerJoints(entt::registry &registry) {
    auto pattern_none_view = registry.view<const FingerPattern>().proxy();
    for (auto &&[entity, pattern]: pattern_none_view) {
        if (pattern.value != FingerPatternType::None) continue;
        PLOG_DEBUG << "Add no finger pattern type.";
        registry.emplace<NoFingerPatternType>(entity);
    }
}

void tagConstantFingerCountJoints(entt::registry &registry) {
    auto pattern_constant_count_view = registry.view<const FingerPattern>().proxy();
    for (auto &&[entity, pattern]: pattern_constant_count_view) {
        if (pattern.value != FingerPatternType::ConstantCount) continue;
        PLOG_DEBUG << "Add constant finger pattern type.";
        registry.emplace<ConstantFingerPatternType>(entity);
    }
}

void tagConstantFingerWidthJoints(entt::registry &registry) {
    auto pattern_constant_width_view = registry.view<const FingerPattern>().proxy();
    for (auto &&[entity, pattern]: pattern_constant_width_view) {
        if (pattern.value != FingerPatternType::ConstantWidth) continue;
        PLOG_DEBUG << "Add constant finger pattern type.";
        registry.emplace<ConstantFingerPatternType>(entity);
    }
}

void tagAutomaticFingerWidthJoints(entt::registry &registry) {
    auto pattern_automatic_view = registry.view<const FingerPattern>().proxy();
    for (auto &&[entity, pattern]: pattern_automatic_view) {
        if (pattern.value != FingerPatternType::AutomaticWidth) continue;
        PLOG_DEBUG << "Add automatic finger pattern type.";
        registry.emplace<AutomaticFingerPatternType>(entity);
    }
}