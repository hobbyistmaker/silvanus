//
// Created by Hobbyist Maker on 9/15/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "updateThicknessDimension.hpp"

#include "entities/DialogInputs.hpp"
#include "entities/PanelThickness.hpp"

#include <plog/Log.h>

using namespace silvanus::generatebox::entities;

void updatePanelThicknessImpl(entt::registry& registry) {
    updateThicknessDimensionInputsImpl(registry);
    updateThicknessDimensionsImpl(registry);
}

void updateThicknessDimensionInputsImpl(entt::registry& registry) {
    auto default_thickness = registry.ctx<DialogThicknessInput>().control;

    auto view = registry.view<PanelThicknessActive, const PanelOverrideInput, const PanelThicknessInput>();
    for (auto &&[entity, active, override, thickness]: view.proxy()) {
        auto toggle = override.control->value() && override.control->isEnabled() ? thickness.control : default_thickness;

        PLOG_DEBUG << "Making " << toggle->id() << " the active thickness input with value " << std::to_string(toggle->value()) << ".";
        active.control = toggle;
    };
}

void updateThicknessDimensionsImpl(entt::registry& registry) {
    auto view = registry.view<PanelThickness, const PanelThicknessActive>();
    for (auto &&[entity, thickness, active]: view.proxy()) {
        PLOG_DEBUG << "Updating joint thickness dimension to " << std::to_string(active.control->value());
        thickness.value = active.control->value();
    }
}

