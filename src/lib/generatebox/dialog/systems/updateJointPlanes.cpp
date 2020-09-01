//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "DialogInputs.hpp"

#include <entt/entt.hpp>
#include <plog/Log.h>

using namespace silvanus::generatebox::entities;

void updateJointPlanesImpl(entt::registry& registry) {
    PLOG_DEBUG << "updating joint planes";
    auto index = registry.ctx<DialogJointIndex>();

    auto view = registry.view<DialogPanelPlanes>().proxy();
    for (auto &&[parent_entity, planes]: view) {
        for (auto const& entity: index.first_panels[parent_entity]) {
            PLOG_DEBUG << "replacing first planes";
            registry.replace<DialogFirstPlanes>(entity, planes);
        }
        for (auto const& entity: index.second_panels[parent_entity]) {
            PLOG_DEBUG << "replacing second planes";
            registry.replace<DialogSecondPlanes>(entity, planes);
        }
    }

    auto first_view = registry.view<DialogFirstPlanes, DialogJoints>().proxy();
    for (auto &&[entity, first, joints]: first_view) {
        PLOG_DEBUG << "updating first planes";
        joints.first.planes = first.planes;
    }

    auto second_view = registry.view<DialogSecondPlanes, DialogJoints>().proxy();
    for (auto &&[entity, second, joints]: second_view) {
        PLOG_DEBUG << "updating second planes";
        joints.second.planes = second.planes;
    }
    PLOG_DEBUG << "finished updating joint planes";
}