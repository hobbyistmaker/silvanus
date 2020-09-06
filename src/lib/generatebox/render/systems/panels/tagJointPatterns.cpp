//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/JointPattern.hpp"
#include "entities/JointPatternTags.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void tagTrimPatternJoints(entt::registry &registry) {
    auto view = registry.view<const JointPattern>().proxy();
    for (auto &&[entity, pattern]: view) {
        if (pattern.value != JointPatternType::Trim) continue;
        PLOG_DEBUG << "Updating TrimJointPattern";
        registry.emplace<TrimJointPattern>(entity);
    }
}

void tagQuadTenonPatternJoints(entt::registry &registry) {
    auto view = registry.view<const JointPattern>().proxy();
    for (auto &&[entity, pattern]: view) {
        if (pattern.value != JointPatternType::QuadTenon) continue;
        PLOG_DEBUG << "Updating QuadTenonJointPattern";
        registry.emplace<QuadTenonJointPattern>(entity);
    }
}

void tagTripleTenonPatternJoints(entt::registry &registry) {
    auto view = registry.view<const JointPattern>().proxy();
    for (auto &&[entity, pattern]: view) {
        if (pattern.value != JointPatternType::TripleTenon) continue;
        PLOG_DEBUG << "Updating TripleTenonJointPattern";
        registry.emplace<TripleTenonJointPattern>(entity);
    }
}

void tagDoubleTenonPatternJoints(entt::registry &registry) {
    auto view = registry.view<const JointPattern>().proxy();
    for (auto &&[entity, pattern]: view) {
        if (pattern.value != JointPatternType::DoubleTenon) continue;
        PLOG_DEBUG << "Updating DoubleTenontJointPattern";
        registry.emplace<DoubleTenonJointPattern>(entity);
    }
}

void tagTenonPatternJoints(entt::registry &registry) {
    auto view = registry.view<const JointPattern>().proxy();
    for (auto &&[entity, pattern]: view) {
        if (pattern.value != JointPatternType::Tenon) continue;
        PLOG_DEBUG << "Updating TenonJointPattern";
        registry.emplace<TenonJointPattern>(entity);
    }
}

void tagLapJointPatternJoints(entt::registry &registry) {
    auto view = registry.view<const JointPattern>().proxy();
    for (auto &&[entity, pattern]: view) {
        if (pattern.value != JointPatternType::LapJoint) continue;
        PLOG_DEBUG << "Updating LapJointPattern";
        registry.emplace<LapJointPattern>(entity);
    }
}

void tagBoxJointPatternJoints(entt::registry &registry) {
    auto view = registry.view<const JointPattern>().proxy();
    for (auto &&[entity, pattern]: view) {
        if (pattern.value != JointPatternType::BoxJoint) continue;
        PLOG_DEBUG << "Updating BoxJointPattern";
        registry.emplace<BoxJointPattern>(entity);
    }
}