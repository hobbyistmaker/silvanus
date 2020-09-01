//
// Created by Hobbyist Maker on 9/5/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_UPDATEJOINTPATTERNINPUTS_HPP
#define SILVANUSPRO_UPDATEJOINTPATTERNINPUTS_HPP

#include "entities/JointPattern.hpp"

#include <entt/entt.hpp>
#include <plog/Log.h>

using namespace silvanus::generatebox::entities;

template <class T, class P>
void updateJointPatternInputsImpl(entt::registry& registry) {
    auto joint_pattern_view = registry.view<T>().proxy();
    for (auto &&[entity, dest]: joint_pattern_view) {
        auto control = registry.ctx<P>().control;
        PLOG_DEBUG << "Adding Joint Pattern Input to entity " << (int)entity << " with value: " << control->selectedItem()->index();
        registry.emplace<DialogJointPatternInput>(entity, control);
    }
}

#endif //SILVANUSPRO_UPDATEJOINTPATTERNINPUTS_HPP
