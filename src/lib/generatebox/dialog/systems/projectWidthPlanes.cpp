//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "DialogSystemManager.hpp"

#include "entities/DialogInputs.hpp"
#include "entities/OrientationTags.hpp"
#include "entities/PanelDimension.hpp"

#include <plog/Log.h>

using namespace silvanus::generatebox::entities;

void projectWidthPlanesImpl(entt::registry& registry) {
    PLOG_DEBUG << "CreateDialog::projectWidthPlane";

    auto length_view = registry.view<DialogPanelPlanes, LengthOrientation, DialogPanelThickness, PanelDimensions, DialogPanel>().proxy();
    for (auto &&[entity, planes, orientation, thickness, dimensions, panel]: length_view) {
        planes.width.max_x = round(dimensions.length * 100000) / 100000;
        planes.width.max_y = round(dimensions.height * 100000) / 100000;
        planes.width.min_x = round((planes.width.max_x - thickness.control->value()) * 100000) / 100000;
        planes.width.min_y = 0;
        PLOG_DEBUG << panel.name << " width plane: (" << planes.width.min_x << ", " << planes.width.min_y << ") to (" << planes.width.max_x << ", "
                   << planes.width.max_y << ")";
    }

    auto width_view = registry.view<DialogPanelPlanes, WidthOrientation, DialogPanelThickness, PanelDimensions, DialogPanel>().proxy();
    for (auto &&[entity, planes, orientation, thickness, dimensions, panel]: width_view) {
        planes.width.max_x = round(dimensions.length * 100000) / 100000;
        planes.width.max_y = round(dimensions.height * 100000) / 100000;
        planes.width.min_x = 0;
        planes.width.min_y = 0;
        PLOG_DEBUG << panel.name << " width plane: (" << planes.width.min_x << ", " << planes.width.min_y << ") to (" << planes.width.max_x << ", "
                   << planes.width.max_y << ")";
    }

    auto height_view = registry.view<DialogPanelPlanes, HeightOrientation, DialogPanelThickness, PanelDimensions, DialogPanel>().proxy();
    for (auto &&[entity, planes, orientation, thickness, dimensions, panel]: height_view) {
        planes.width.max_x = round(dimensions.length * 100000) / 100000;
        planes.width.max_y = round(dimensions.height * 100000) / 100000;
        planes.width.min_x = 0;
        planes.width.min_y = round((planes.width.max_y - thickness.control->value()) * 100000) / 100000;
        PLOG_DEBUG << panel.name << " width plane: (" << planes.width.min_x << ", " << planes.width.min_y << ") to (" << planes.width.max_x << ", "
                   << planes.width.max_y << ")";
    }
}