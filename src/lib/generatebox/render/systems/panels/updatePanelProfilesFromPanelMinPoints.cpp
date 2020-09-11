//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include "plog/Log.h"
#include "entities/PanelProfile.hpp"
#include "entities/OrientationTags.hpp"
#include "entities/PanelMaxPoint.hpp"

using namespace silvanus::generatebox::entities;

void updatePanelProfilesValues(entt::registry &registry) {
    auto length_view = registry.view<PanelProfile, const LengthOrientation, const PanelMaxPoint, const PanelMaxParam>().proxy();
    for (auto &&[entity, profile, orientation, reference, reference_param]: length_view) {
        PLOG_DEBUG << "Updating LengthOrientation panel profile";
        profile.length.value = reference.width;
        profile.length.expression = reference_param.width;
        profile.width.value = reference.height;
        profile.width.expression = reference_param.height;
    }

    auto width_view = registry.view<PanelProfile, const WidthOrientation, const PanelMaxPoint, const PanelMaxParam>().proxy();
    for (auto &&[entity, profile, orientation, reference, reference_param]: width_view) {
        PLOG_DEBUG << "Updating WidthOrientation panel profile";
        profile.length.value = reference.length;
        profile.length.expression = reference_param.length;
        profile.width.value = reference.height;
        profile.width.expression = reference_param.height;
    }

    auto height_view = registry.view<PanelProfile, const HeightOrientation, const PanelMaxPoint, const PanelMaxParam>().proxy();
    for (auto &&[entity, profile, orientation, reference, reference_param]: height_view) {
        PLOG_DEBUG << "Updating HeightOrientation panel profile";
        profile.length.value = reference.length;
        profile.length.expression = reference_param.length;
        profile.width.value = reference.width;
        profile.width.expression = reference_param.width;
    }
}

void updatePanelProfilesExpressions(entt::registry &registry) {
    auto length_view = registry.view<PanelProfileParams, const LengthOrientation, const PanelMaxParam>().proxy();
    for (auto &&[entity, profile, orientation, reference]: length_view) {
        profile.length = reference.width;
        profile.width = reference.height;
        PLOG_DEBUG << "Updating LengthOrientation panel profile expressions: " << profile.length << ", " << profile.width;
    }

    auto width_view = registry.view<PanelProfileParams, const WidthOrientation, const PanelMaxParam>().proxy();
    for (auto &&[entity, profile, orientation, reference]: width_view) {
        profile.length = reference.length;
        profile.width = reference.height;
        PLOG_DEBUG << "Updating WidthOrientation panel profile expressions: " << profile.length << ", " << profile.width;
    }

    auto height_view = registry.view<PanelProfileParams, const HeightOrientation, const PanelMaxParam>().proxy();
    for (auto &&[entity, profile, orientation, reference]: height_view) {
        profile.length = reference.length;
        profile.width = reference.width;
        PLOG_DEBUG << "Updating HeightOrientation panel profile expressions: " << profile.length << ", " << profile.width;
    }
}

void updatePanelProfilesFromPanelMinPoints(entt::registry& registry) {
    updatePanelProfilesValues(registry);
    updatePanelProfilesExpressions(registry);
}
