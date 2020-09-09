//
// Created by Hobbyist Maker on 9/6/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_UPDATEJOINTDIRECTIONINPUTS_HPP
#define SILVANUSPRO_UPDATEJOINTDIRECTIONINPUTS_HPP

#include "lib/generatebox/dialog/entities/DialogInputs.hpp"
#include "entities/JointDirection.hpp"
#include "entities/JointPosition.hpp"
#include "entities/PanelPosition.hpp"

#include <entt/entt.hpp>
#include <plog/Log.h>

using namespace silvanus::generatebox::entities;

template <class T, class P>
void updateInsideJointDirectionInputsImpl(entt::registry& registry, bool reverse = false) {
    PLOG_DEBUG << "starting updateInsideJointDirectionInputsImpl";
    auto view = registry.view<T, Panel, PanelPositions>();
    for (auto &&[entity, dest, panel, positions]: view.proxy()) {
        PLOG_DEBUG << panel.name << " positions are: " << (int)positions.first << ":" << (int)positions.second;
        if (positions.first == Position::Outside || positions.second == Position::Outside) continue;

        auto control = registry.ctx<P>().control;
        PLOG_DEBUG << "Adding Joint Direction Input to entity " << (int)entity << " with value: " << control->selectedItem()->index();
        registry.emplace<DialogJointDirectionInputs>(entity, DialogJointDirectionInput{control, reverse}, DialogJointDirectionInput{control, !reverse});
    }
}

#endif //SILVANUSPRO_UPDATEJOINTDIRECTIONINPUTS_HPP
