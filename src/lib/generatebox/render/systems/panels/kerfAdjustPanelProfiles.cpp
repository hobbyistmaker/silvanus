//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include <entt/entt.hpp>
#include <plog/Log.h>
#include <fmt/format.h>

#include "entities/PanelProfile.hpp"
#include "entities/Kerf.hpp"

using namespace silvanus::generatebox::entities;

void kerfAdjustPanelProfilesValues(entt::registry& registry) {
    auto kerf_view = registry.view<PanelProfile, const Kerf,const KerfParam>().proxy();
    for (auto &&[entity, profile, kerf, kerf_param]: kerf_view) {
        PLOG_DEBUG << "Adjusting panel profile kerf";
        auto kerf_expr = fmt::format(" + {0}", kerf_param.expression);

        auto has_length = profile.length.expression.length() > 0;
        auto has_width = profile.width.expression.length() > 0;

        auto length_kerf_expr = has_length ? kerf_expr : "";
        auto width_kerf_expr = has_width ? kerf_expr : "";

        auto length_expr = has_length ? fmt::format("{0}{1}", profile.length.expression, kerf_expr) : "";
        auto width_expr = has_width ? fmt::format("{0}{1}", profile.width.expression, kerf_expr) : "";

        profile.length.value += kerf.value;
        profile.width.value += kerf.value;

        profile.length.expression = length_expr;
        profile.width.expression = width_expr;
    }
}

void kerfAdjustPanelProfilesExpressions(entt::registry& registry) {
    auto kerf_view = registry.view<PanelProfileParams, const KerfParam>().proxy();
    for (auto &&[entity, profile, kerf_param]: kerf_view) {
        auto kerf_expr = fmt::format(" + {0}", kerf_param.expression);

        auto has_length = profile.length.length() > 0;
        auto has_width = profile.width.length() > 0;

        auto length_kerf_expr = has_length ? kerf_expr : "";
        auto width_kerf_expr = has_width ? kerf_expr : "";

        auto length_expr = has_length ? fmt::format("{0}{1}", profile.length, kerf_expr) : "";
        auto width_expr = has_width ? fmt::format("{0}{1}", profile.width, kerf_expr) : "";

        profile.length = length_expr;
        profile.width = width_expr;
        PLOG_DEBUG << "Adjusting panel profile kerf expressions: " << profile.length << ", " << profile.width;
    }
}

void kerfAdjustPanelProfiles(entt::registry& registry) {
    PLOG_DEBUG << "Started kerfAdjustPanelProfiles";
    kerfAdjustPanelProfilesValues(registry);
    kerfAdjustPanelProfilesExpressions(registry);
    PLOG_DEBUG << "Finished kerfAdjustPanelProfiles";
}