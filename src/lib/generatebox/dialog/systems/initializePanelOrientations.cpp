//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/OrientationTags.hpp"
#include "entities/PanelOrientation.hpp"

#include "systems/addPanelOrientation.hpp"

#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

void initializePanelOrientationsImpl(entt::registry& registry) {
    addPanelOrientation<LengthOrientation, AxisFlag::Length>(registry);
    addPanelOrientation<WidthOrientation, AxisFlag::Width>(registry);
    addPanelOrientation<HeightOrientation, AxisFlag::Height>(registry);
}