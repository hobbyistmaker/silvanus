//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>

#include "entities/EndReferencePoint.hpp"
#include "entities/Dimensions.hpp"

using namespace silvanus::generatebox::entities;

void updateEndReferencePointsFromDimensions(entt::registry& registry) {
    PLOG_DEBUG << "Started updateEndReferencePoints";
    auto view = registry.view<EndReferencePoint, const Dimensions>().proxy();

    for (auto &&[entity, reference, dimensions]: view) {
        PLOG_DEBUG << "Updating end reference points";
        reference.length.value = dimensions.length;
        reference.width.value = dimensions.width;
        reference.height.value = dimensions.height;
    }
    PLOG_DEBUG << "Finished updateEndReferencePoints";
}