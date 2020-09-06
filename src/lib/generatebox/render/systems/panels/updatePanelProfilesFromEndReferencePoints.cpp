//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include "plog/Log.h"
#include "entities/PanelProfile.hpp"
#include "entities/OrientationTags.hpp"
#include "entities/EndReferencePoint.hpp"

using namespace silvanus::generatebox::entities;

void updatePanelProfilesFromEndReferencePoints(entt::registry& registry) {
    auto length_view = registry.view<PanelProfile, const LengthOrientation, const EndReferencePoint>().proxy();
    for (auto &&[entity, profile, orientation, reference]: length_view) {
        PLOG_DEBUG << "Updating LengthOrientation panel profile";
        profile.length.value = reference.width.value;
        profile.width.value = reference.height.value;
    }

    auto width_view = registry.view<PanelProfile, const WidthOrientation, const EndReferencePoint>().proxy();
    for (auto &&[entity, profile, orientation, reference]: width_view) {
        PLOG_DEBUG << "Updating WidthOrientation panel profile";
        profile.length.value = reference.length.value;
        profile.width.value = reference.height.value;
    }

    auto height_view = registry.view<PanelProfile, const HeightOrientation, const EndReferencePoint>().proxy();
    for (auto &&[entity, profile, orientation, reference]: height_view) {
        PLOG_DEBUG << "Updating HeightOrientation panel profile";
        profile.length.value = reference.length.value;
        profile.width.value = reference.width.value;
    }
}