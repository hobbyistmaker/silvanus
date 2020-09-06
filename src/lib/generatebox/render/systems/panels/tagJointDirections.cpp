//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/JointDirection.hpp"

#include <plog/Log.h>
#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void tagInverseDirectionJoints(entt::registry &registry) {
    auto direction_inverse_view = registry.view<const JointDirection>().proxy();
    for (auto &&[entity, direction]: direction_inverse_view) {
        if (direction.value != JointDirectionType::Inverted) continue;
        PLOG_DEBUG << "Add inverse joint direction.";
        registry.emplace<InverseJointDirection>(entity);
    }
}

void tagNormalDirectionJoints(entt::registry &registry) {
    auto direction_normal_view = registry.view<const JointDirection>().proxy();
    for (auto &&[entity, direction]: direction_normal_view) {
        if (direction.value != JointDirectionType::Normal) continue;
        PLOG_DEBUG << "Add normal joint direction.";
        registry.emplace<NormalJointDirection>(entity);
    }
}