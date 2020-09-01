//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_ADDPANELDIMENSIONS_HPP
#define SILVANUSPRO_ADDPANELDIMENSIONS_HPP

#include "entities/PanelDimension.hpp"

#include <entt/entt.hpp>

#include <plog/Log.h>

using silvanus::generatebox::entities::PanelDimensionInputs;

template<class L, class W, class H>
void addPanelDimensions(entt::registry& registry, entt::entity entity) {
    auto length = registry.ctx<L>().control;
    auto width  = registry.ctx<W>().control;
    auto height = registry.ctx<H>().control;

    PLOG_DEBUG << "Panel dimensions: " << length->value() << ", " << width->value() << ", " << height->value();
    registry.emplace<PanelDimensionInputs>(entity, length, width, height);
}

#endif //SILVANUSPRO_ADDPANELDIMENSIONS_HPP
