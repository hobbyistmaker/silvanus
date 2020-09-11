//
// Created by Hobbyist Maker on 9/11/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_VALIDATEDIMENSIONS_HPP
#define SILVANUSPRO_VALIDATEDIMENSIONS_HPP

#include "entities/EntitiesAll.hpp"
#include <entt/entt.hpp>

using silvanus::generatebox::entities::AxisFlag;
using silvanus::generatebox::entities::PanelEnableInput;
using silvanus::generatebox::entities::PanelThicknessInput;
using silvanus::generatebox::entities::DialogErrorMessage;

template<class T, class U>
bool validateDimension(entt::registry &m_configuration, AxisFlag axis_flag) {
    auto msg_map = std::unordered_map<AxisFlag, std::string>{
        {AxisFlag::Height, "<span style=\" font-weight:600; color:#ff0000;\">Height</span> should be greater than the thickness of top to bottom panels."},
        {AxisFlag::Length, "<span style=\" font-weight:600; color:#ff0000;\">Length</span> should be greater than the thickness of left to right panels."},
        {AxisFlag::Width,  "<span style=\" font-weight:600; color:#ff0000;\">Width</span> should be greater than the thickness of front to back panels."},
    };

    auto dimension_control = m_configuration.ctx<T>().control;

    auto minimum = 0.0;

    auto view = m_configuration.view<const U, const PanelEnableInput, const PanelThicknessInput>().proxy();
    for (auto &&[entity, dimension, enable, thickness]: view) {
        minimum += thickness.control->value() * enable.control->value();
    }

    auto value = dimension_control->value();
    if (value > minimum) { return true; }

    m_configuration.set<DialogErrorMessage>(msg_map[axis_flag]);

    return false;
}

#endif //SILVANUSPRO_VALIDATEDIMENSIONS_HPP
