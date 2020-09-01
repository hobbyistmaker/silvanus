//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/FingerWidth.hpp"
#include "entities/FingerWidthInput.hpp"

#include "DialogSystemManager.hpp"

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::dialog;

void updateFingerWidthImpl(entt::registry &registry) {
    auto finger_width_view = registry.view<FingerWidth, const FingerWidthInput>().proxy();
    for (auto &&[entity, width, input]: finger_width_view) {
        width.value = input.control->value();
    }
}
