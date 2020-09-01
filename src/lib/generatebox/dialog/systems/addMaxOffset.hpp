//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_ADDMAXOFFSET_HPP
#define SILVANUSPRO_ADDMAXOFFSET_HPP

#include "entities/MaxOffsetInput.hpp"

#include <entt/entt.hpp>

using silvanus::generatebox::entities::MaxOffsetInput;

template<class M>
void addMaxOffset(entt::registry& registry, entt::entity entity) {
    auto offset = registry.ctx<M>().control;

    registry.emplace<MaxOffsetInput>(entity, offset);
}

#endif //SILVANUSPRO_ADDMAXOFFSET_HPP
