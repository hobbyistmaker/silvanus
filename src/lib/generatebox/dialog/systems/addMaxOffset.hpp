//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_ADDMAXOFFSET_HPP
#define SILVANUSPRO_ADDMAXOFFSET_HPP

#include "entities/MaxOffset.hpp"

#include <entt/entt.hpp>

using silvanus::generatebox::entities::MaxOffsetInput;

template<class H>
void addHeightMaxOffset(entt::registry& registry, entt::entity entity) {
    auto offset = registry.ctx<H>().control;

    registry.emplace<HeightMaxOffsetInput>(entity, offset);
}

template<class L>
void addLengthMaxOffset(entt::registry& registry, entt::entity entity) {
    auto offset = registry.ctx<L>().control;

    registry.emplace<LengthMaxOffsetInput>(entity, offset);
}

template<class W>
void addWidthMaxOffset(entt::registry& registry, entt::entity entity) {
    auto offset = registry.ctx<W>().control;

    registry.emplace<WidthMaxOffsetInput>(entity, offset);
}

#endif //SILVANUSPRO_ADDMAXOFFSET_HPP
