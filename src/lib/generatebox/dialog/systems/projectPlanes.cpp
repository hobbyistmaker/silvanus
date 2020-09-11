//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "projectPlanes.hpp"

#include "entities/DialogInputs.hpp"
#include "entities/Panel.hpp"
#include "entities/PanelAxis.hpp"
#include "entities/PanelMaxPoint.hpp"
#include "entities/PanelThickness.hpp"
#include "entities/Thickness.hpp"

#include <plog/Log.h>

using namespace silvanus::generatebox::entities;


void projectPlanesImpl(entt::registry& registry) {
    projectLengthPlanesImpl(registry);
    projectWidthPlanesImpl(registry);
    projectHeightPlanesImpl(registry);
}

void projectPlaneParamsImpl(entt::registry& registry) {
    projectLengthPlaneParamsImpl(registry);
    projectWidthPlaneParamsImpl(registry);
    projectHeightPlaneParamsImpl(registry);
}

void projectLengthPlanesImpl(entt::registry& registry) {
    PLOG_DEBUG << "CreateDialog::projectLengthPlane";

    auto length_view = registry.view<PanelPlanes, PanelAxis, PanelThicknessActive, PanelMaxPoint, Panel>();
    for (auto &&[entity, planes, orientation, thickness, dimensions, panel]: length_view.proxy()) {

        planes.length.max_x = round(dimensions.width * 100000) / 100000;
        planes.length.max_y = round(dimensions.height * 100000) / 100000;
        planes.length.min_x = (round((planes.length.max_x - thickness.control->value()) * 100000) / 100000) * orientation.width;
        planes.length.min_y = (round((planes.length.max_y - thickness.control->value()) * 100000) / 100000) * orientation.height;

        PLOG_DEBUG << panel.name << " length plane: (" << planes.length.min_x << ", " << planes.length.min_y << ") to (" << planes.length.max_x << ", "
                   << planes.length.max_y << ")";
    }
}

void projectWidthPlanesImpl(entt::registry& registry) {
    PLOG_DEBUG << "CreateDialog::projectWidthPlane";

    auto length_view = registry.view<PanelPlanes, PanelAxis, PanelThicknessActive, PanelMaxPoint, Panel>();
    for (auto &&[entity, planes, orientation, thickness, dimensions, panel]: length_view.proxy()) {

        planes.width.max_x = round(dimensions.length * 100000) / 100000;
        planes.width.max_y = round(dimensions.height * 100000) / 100000;
        planes.width.min_x = (round((planes.width.max_x - thickness.control->value()) * 100000) / 100000) * orientation.length;
        planes.width.min_y = (round((planes.width.max_y - thickness.control->value()) * 100000) / 100000) * orientation.height;

        PLOG_DEBUG << panel.name << " width plane: (" << planes.width.min_x << ", " << planes.width.min_y << ") to (" << planes.width.max_x << ", "
                   << planes.width.max_y << ")";
    }
}

void projectHeightPlanesImpl(entt::registry& registry) {
    PLOG_DEBUG << "CreateDialog::projectHeightPlane";

    auto length_view = registry.view<PanelPlanes, PanelAxis, PanelThicknessActive, PanelMaxPoint, Panel>();
    for (auto &&[entity, planes, orientation, thickness, dimensions, panel]: length_view.proxy()) {

        planes.height.max_x = round(dimensions.length * 100000) / 100000;
        planes.height.max_y = round(dimensions.width * 100000) / 100000;
        planes.height.min_x = (round((planes.height.max_x - thickness.control->value()) * 100000) / 100000) * orientation.length;
        planes.height.min_y = (round((planes.height.max_y - thickness.control->value()) * 100000) / 100000) * orientation.width;

        PLOG_DEBUG << panel.name << " height plane: (" << planes.height.min_x << ", " << planes.height.min_y << ") to (" << planes.height.max_x << ", "
                   << planes.height.max_y << ")";
    }
}

void projectLengthPlaneParamsImpl(entt::registry& registry) {
    PLOG_DEBUG << "CreateDialog::projectLengthPlaneParamsImpl";

    auto length_view = registry.view<PanelPlanesParams, PanelAxis, ThicknessParameter, PanelMaxParam, Panel>();
    for (auto &&[entity, planes, orientation, thickness, dimensions, panel]: length_view.proxy()) {

        planes.length.max_x = dimensions.width;
        planes.length.max_y = dimensions.height;
        planes.length.min_x = orientation.width ? planes.length.max_x + " - " + thickness.name : "";
        planes.length.min_y = orientation.height ? planes.length.max_y + " - " + thickness.name : "";

        PLOG_DEBUG << panel.name << " length plane: (" << planes.length.min_x << ", " << planes.length.min_y << ") to (" << planes.length.max_x << ", "
                   << planes.length.max_y << ")";
    }

    auto fix_view = registry.view<PanelPlanesParams, ThicknessParameter, Panel>();
    for (auto &&[entity, planes, thickness, panel]: fix_view.proxy()) {
        auto invalid = thickness.name + " - " + thickness.name;
        planes.length.min_x = planes.length.min_x == invalid ? "" : planes.length.min_x;
        planes.length.min_y = planes.length.min_y == invalid ? "" : planes.length.min_y;

        PLOG_DEBUG << panel.name << " length plane: (" << planes.length.min_x << ", " << planes.length.min_y << ") to (" << planes.length.max_x << ", "
                   << planes.length.max_y << ")";
    }
}

void projectWidthPlaneParamsImpl(entt::registry& registry) {
    PLOG_DEBUG << "CreateDialog::projectWidthPlaneParamsImpl";

    auto length_view = registry.view<PanelPlanesParams, PanelAxis, ThicknessParameter, PanelMaxParam, Panel>();
    for (auto &&[entity, planes, orientation, thickness, dimensions, panel]: length_view.proxy()) {

        planes.width.max_x = dimensions.length;
        planes.width.max_y = dimensions.height;
        planes.width.min_x = orientation.length ? planes.width.max_x + " - " + thickness.name : "";
        planes.width.min_y = orientation.height ? planes.width.max_y + " - " + thickness.name : "";

        PLOG_DEBUG << panel.name << " width plane: (" << planes.width.min_x << ", " << planes.width.min_y << ") to (" << planes.width.max_x << ", "
                   << planes.width.max_y << ")";
    }

    auto fix_view = registry.view<PanelPlanesParams, ThicknessParameter, Panel>();
    for (auto &&[entity, planes, thickness, panel]: fix_view.proxy()) {
        auto invalid = thickness.name + " - " + thickness.name;
        planes.width.min_x = planes.width.min_x == invalid ? "" : planes.width.min_x;
        planes.width.min_y = planes.width.min_y == invalid ? "" : planes.width.min_y;

        PLOG_DEBUG << panel.name << " width plane: (" << planes.width.min_x << ", " << planes.width.min_y << ") to (" << planes.width.max_x << ", "
                   << planes.width.max_y << ")";
    }
}

void projectHeightPlaneParamsImpl(entt::registry& registry) {
    PLOG_DEBUG << "CreateDialog::projectHeightPlaneParamsImpl";

    auto length_view = registry.view<PanelPlanesParams, PanelAxis, ThicknessParameter, PanelMaxParam, Panel>();
    for (auto &&[entity, planes, orientation, thickness, dimensions, panel]: length_view.proxy()) {

        planes.height.max_x = dimensions.length;
        planes.height.max_y = dimensions.width;
        planes.height.min_x = orientation.length ? planes.height.max_x + " - " + thickness.name : "";
        planes.height.min_y = orientation.width ? planes.height.max_y + " - " + thickness.name : "";

        PLOG_DEBUG << panel.name << " height plane: (" << planes.height.min_x << ", " << planes.height.min_y << ") to (" << planes.height.max_x << ", "
                   << planes.height.max_y << ")";
    }

    auto fix_view = registry.view<PanelPlanesParams, ThicknessParameter, Panel>();
    for (auto &&[entity, planes, thickness, panel]: fix_view.proxy()) {
        auto invalid = thickness.name + " - " + thickness.name;
        planes.height.min_x = planes.height.min_x == invalid ? "" : planes.height.min_x;
        planes.height.min_y = planes.height.min_y == invalid ? "" : planes.height.min_y;

        PLOG_DEBUG << panel.name << " height plane: (" << planes.height.min_x << ", " << planes.height.min_y << ") to (" << planes.height.max_x << ", "
                   << planes.height.max_y << ")";
    }
}
