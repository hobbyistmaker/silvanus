//
// Created by Hobbyist Maker on 9/12/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_ENTITY_HELPERS_HPP
#define SILVANUSPRO_ENTITY_HELPERS_HPP

#include "entities/AxisFlag.hpp"
#include "entities/Position.hpp"

#include <entt/entt.hpp>

namespace silvanus::generatebox {

    auto makeDialogPanelEntity(entt::registry& registry, entities::Position position, std::string name, int priority, entities::AxisFlag orientation) -> entt::entity;
    auto makePanelEntity(entt::registry& registry) -> entt::entity;
    auto makeJointEntity(entt::registry& registry) -> entt::entity;

    void maxWidthPanel(entt::registry& registry, entt::entity entity);
    void minWidthPanel(entt::registry& registry, entt::entity entity);

    void maxLengthPanel(entt::registry& registry, entt::entity entity);
    void minLengthPanel(entt::registry& registry, entt::entity entity);

    void maxHeightPanel(entt::registry& registry, entt::entity entity);
    void minHeightPanel(entt::registry& registry, entt::entity entity);

}

#endif //SILVANUSPRO_ENTITY_HELPERS_HPP
