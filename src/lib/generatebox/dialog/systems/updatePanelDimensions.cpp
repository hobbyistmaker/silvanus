//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "DialogSystemManager.hpp"

#include "entities/PanelDimension.hpp"

#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void updatePanelDimensionsImpl(entt::registry& registry) {
    registry.view<PanelDimensions, PanelDimensionInputs>().each([](
        auto &dimensions, auto const &inputs
    ){
        dimensions.length = inputs.length->value();
        dimensions.width = inputs.width->value();
        dimensions.height = inputs.height->value();
    });
}