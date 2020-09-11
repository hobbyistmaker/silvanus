//
// Created by Hobbyist Maker on 9/18/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//


#include <entt/entt.hpp>
#include <plog/Log.h>

#include "entities/JointPanel.hpp"
#include "entities/JointPanelOffset.hpp"
#include "entities/Kerf.hpp"
#include "entities/PanelPosition.hpp"
#include "entities/Position.hpp"

using namespace silvanus::generatebox::entities;

void updateJointPanelOffsetsFromExpressions(entt::registry& registry) {
    auto view = registry.view<JointPanelOffset, const JointPanelOffsetParam>();

    for (auto &&[entity, offset, param]: view.proxy()) {
        offset.expression = param.expression;
        PLOG_DEBUG << (int)entity << ": Updating joint panel offset expression to " << offset.expression;
    }
}
