//
// Created by Hobbyist Maker on 9/6/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_UPDATEJOINTDIRECTIONINPUTS_HPP
#define SILVANUSPRO_UPDATEJOINTDIRECTIONINPUTS_HPP

#include "entities/DialogInputs.hpp"
#include "entities/JointDirection.hpp"
#include "entities/JointPosition.hpp"
#include "entities/PanelPosition.hpp"

#include <entt/entt.hpp>
#include <plog/Log.h>

using namespace silvanus::generatebox::entities;

template <class T, class P>
void updateJointDirectionInputsImpl(entt::registry& registry) {
    auto view = registry.view<T, PanelPosition, JointPosition>();
    for (auto &&[entity, dest, panel, joint]: view.proxy()) {
        if (!(panel.value == Position::Inside && joint.value == Position::Inside)) continue;

        auto control = registry.ctx<P>().control;
        PLOG_DEBUG << "Adding Joint Direction Input to entity " << (int)entity << " with value: " << control->selectedItem()->index();
        registry.emplace<DialogJointDirectionInputs>(entity, control);
    }
}

#endif //SILVANUSPRO_UPDATEJOINTDIRECTIONINPUTS_HPP
