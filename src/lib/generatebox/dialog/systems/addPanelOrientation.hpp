//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_ADDPANELORIENTATION_HPP
#define SILVANUSPRO_ADDPANELORIENTATION_HPP

#include "entities/AxisFlag.hpp"
#include "entities/DialogInputs.hpp"

#include <entt/entt.hpp>

using silvanus::generatebox::entities::AxisFlag;
using silvanus::generatebox::entities::DialogPanel;

template <class T, AxisFlag A>
void addPanelOrientation(entt::registry& registry) {
    registry.view<DialogPanel>().each([&](
        auto entity, auto const& panel
    ){
        if (panel.orientation != A) return;
        registry.emplace<T>(entity);
    });
}

#endif //SILVANUSPRO_ADDPANELORIENTATION_HPP
