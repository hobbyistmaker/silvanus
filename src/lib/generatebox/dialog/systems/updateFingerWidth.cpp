//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "updateFingerWidth.hpp"

#include "entities/FingerWidth.hpp"

using namespace silvanus::generatebox::entities;

void updateFingerWidthImpl(entt::registry &registry) {
    auto finger_width_view = registry.view<FingerWidth, const FingerWidthInput>().proxy();
    for (auto &&[entity, width, input]: finger_width_view) {
        width.value = input.control->value();
    }
}
