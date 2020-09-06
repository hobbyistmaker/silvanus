//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>
#include "entities/PanelProfile.hpp"
#include "entities/Kerf.hpp"

using namespace silvanus::generatebox::entities;

void kerfAdjustPanelProfiles(entt::registry& registry) {
    PLOG_DEBUG << "Started kerfAdjustPanelProfiles";

    auto kerf_view = registry.view<PanelProfile, const Kerf>().proxy();
    for (auto &&[entity, profile, kerf]: kerf_view) {
        PLOG_DEBUG << "Adjusting panel profile kerf";
        profile.length.value += kerf.value;
        profile.width.value += kerf.value;
    }
    PLOG_DEBUG << "Finished kerfAdjustPanelProfiles";
}