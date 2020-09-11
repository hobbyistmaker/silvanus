//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "DialogSystemManager.hpp"

#include "entities/PanelDimension.hpp"
#include "entities/PanelMax.hpp"

#include <entt/entt.hpp>
#include <Core/UserInterface/CommandInput.h>

using namespace adsk::core;
using namespace silvanus::generatebox::entities;

using adsk::core::DefaultModelingOrientations::YUpModelingOrientation;

void updatePanelMinPointFromThickness(entt::registry &registry) {
    registry.view<PanelMinPoint, const PanelMaxPoint, const PanelThickness, const PanelAxis>().each([](
        auto &dimensions, auto const& max_point, auto const& thickness, auto const& normal
    ){
        dimensions.length = (max_point.length - (thickness.value * normal.length)) * normal.length;
        dimensions.width = (max_point.width - (thickness.value * normal.width)) * normal.width;
        dimensions.height = (max_point.height - (thickness.value * normal.height)) * normal.height;
        PLOG_DEBUG << "Updated min point is (" << dimensions.length << ", " << dimensions.width << ", " << dimensions.height << ")";
    });

    registry.view<PanelMinParam, const PanelMaxParam, const ThicknessParameter, const PanelAxis>().each([](
       auto &dimensions, auto const& max_point, auto const& thickness, auto const& normal
    ){
        dimensions.length = normal.length ? max_point.length + " - " + thickness.name : dimensions.length;
        dimensions.width  = normal.width  ? max_point.width  + " - " + thickness.name : dimensions.width ;
        dimensions.height = normal.height ? max_point.height + " - " + thickness.name : dimensions.height;
        PLOG_DEBUG << "Updated min point param from thickness and axis is (" << dimensions.length << ", " << dimensions.width << ", " << dimensions.height << ")";
    });

    registry.view<PanelMinParam, const ThicknessParameter>().each([](
       auto &dimensions, auto const& thickness
    ){
        auto invalid = thickness.name + " - " + thickness.name;
        dimensions.length = dimensions.length == invalid ? "" : dimensions.length;
        dimensions.width = dimensions.width == invalid ? "" : dimensions.width;
        dimensions.height = dimensions.height == invalid ? "" : dimensions.height;
        PLOG_DEBUG << "Updated min point param from thickness is (" << dimensions.length << ", " << dimensions.width << ", " << dimensions.height << ")";
    });
}

void updatePanelMaxPointFromHeightInput(entt::registry &registry) {
    registry.view<PanelMaxPoint, PanelMaxHeightInput>().each([](
        auto &dimensions, auto const &height
    ){
        dimensions.height = height.control->value();
        PLOG_DEBUG << "Updated max point is (" << dimensions.length << ", " << dimensions.width << ", " << dimensions.height << ")";
    });

    registry.view<PanelMaxParam, MaxHeightParam>().each([](
        auto &dimensions, auto const &height
    ){
        dimensions.height = height.expression.length() > 0 ? height.expression : dimensions.length;
        PLOG_DEBUG << "Updated max point param from height is (" << dimensions.length << ", " << dimensions.width << ", " << dimensions.height << ")";
    });
}

void updatePanelMaxPointFromWidthInput(entt::registry &registry) {
    registry.view<PanelMaxPoint, PanelMaxWidthInput>().each([](
        auto &dimensions, auto const &width
    ){
        dimensions.width = width.control->value();
        PLOG_DEBUG << "Updated max point is (" << dimensions.length << ", " << dimensions.width << ", " << dimensions.height << ")";
    });

    registry.view<PanelMaxParam, MaxWidthParam>().each([](
        auto &dimensions, auto const &width
    ){
        dimensions.width = width.expression.length() > 0 ? width.expression : dimensions.length;
        PLOG_DEBUG << "Updated max point param from width is (" << dimensions.length << ", " << dimensions.width << ", " << dimensions.height << ")";
    });
}

void updatePanelMaxPointFromLengthInput(entt::registry &registry) {
    registry.view<PanelMaxPoint, PanelMaxLengthInput>().each([](
        auto &dimensions, auto const &length
    ){
        dimensions.length = length.control->value();
        PLOG_DEBUG << "Updated max point from length is (" << dimensions.length << ", " << dimensions.width << ", " << dimensions.height << ")";
    });

    registry.view<PanelMaxParam, MaxLengthParam>().each([](
       auto &dimensions, auto const &length
    ){
        dimensions.length = length.expression.length() > 0 ? length.expression : dimensions.length;
        PLOG_DEBUG << "Updated max point param from length is (" << dimensions.length << ", " << dimensions.width << ", " << dimensions.height << ")";
    });
}

void updatePanelMaxPointFromPanelMaximums(entt::registry &registry) {
    registry.view<PanelMaxPoint, PanelMaximums>().each([](
        auto &dimensions, auto const &maximums
    ){
        dimensions.length = maximums.length;
        dimensions.width = maximums.width;
        dimensions.height = maximums.height;
        PLOG_DEBUG << "Updated max point is (" << dimensions.length << ", " << dimensions.width << ", " << dimensions.height << ")";
    });

    registry.view<PanelMaxParam, PanelMaximumExpressions>().each([](
        auto &dimensions, auto const &maximums
    ){
        dimensions.length = maximums.length.length() > 0 ? maximums.length : dimensions.length;
        dimensions.width = maximums.width.length() > 0 ? maximums.width : dimensions.width;
        dimensions.height = maximums.height.length() > 0 ? maximums.height : dimensions.height;
        PLOG_DEBUG << "Updated max point param is (" << dimensions.length << ", " << dimensions.width << ", " << dimensions.height << ")";
    });
}

void initializePanelMaxPointFromThickness(entt::registry &registry) {
    registry.view<PanelMaxPoint, PanelThickness, PanelAxis>().each([](
        auto &dimensions, auto const& thickness, auto const& normal
    ){
        dimensions.length = thickness.value * normal.length;
        dimensions.width = thickness.value * normal.width;
        dimensions.height = thickness.value * normal.height;
        PLOG_DEBUG << "Updated max point is (" << dimensions.length << ", " << dimensions.width << ", " << dimensions.height << ")";
    });

    registry.view<PanelMaxParam, ThicknessParameter, PanelAxis>().each([](
        auto &dimensions, auto const& thickness, auto const& normal
    ){
        dimensions.length = normal.length ? thickness.name : "";
        dimensions.width = normal.width ? thickness.name : "";
        dimensions.height = normal.height ? thickness.name : "";
        PLOG_DEBUG << "Updated max point param is (" << dimensions.length << ", " << dimensions.width << ", " << dimensions.height << ")";
    });
}

void updatePanelDimensionsImpl(entt::registry& registry) {
    PLOG_DEBUG << "starting updatePanelDimensionsImpl";

    initializePanelMaxPointFromThickness(registry);
    updatePanelMaxPointFromPanelMaximums(registry);
    updatePanelMaxPointFromLengthInput(registry);
    updatePanelMaxPointFromWidthInput(registry);
    updatePanelMaxPointFromHeightInput(registry);
    updatePanelMinPointFromThickness(registry);

    PLOG_DEBUG << "finished updatePanelDimensionsImpl";
}
