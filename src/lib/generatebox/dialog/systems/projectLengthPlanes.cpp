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

void projectLengthPlanesImpl(entt::registry& registry) {
    PLOG_DEBUG << "CreateDialog::projectLengthPlane";

    auto length_view = registry.view<DialogPanelPlanes, LengthOrientation, DialogPanelThickness, PanelDimensions, DialogPanel>().proxy();
    for (auto &&[entity, planes, orientation, thickness, dimensions, panel]: length_view) {
        PLOG_DEBUG << "Thickness = " << thickness.control->value();
        PLOG_DEBUG << panel.name << " dimensions: " << dimensions.width << ", " << dimensions.height;
        planes.length.max_x = round(dimensions.width * 100000) / 100000;
        planes.length.max_y = round(dimensions.height * 100000) / 100000;
        planes.length.min_x = 0;
        planes.length.min_y = 0;
        PLOG_DEBUG << panel.name << " length plane: (" << planes.length.min_x << ", " << planes.length.min_y << ") to (" << planes.length.max_x << ", "
                   << planes.length.max_y << ")";
    }

    auto width_view = registry.view<DialogPanelPlanes, WidthOrientation, DialogPanelThickness, PanelDimensions, DialogPanel>().proxy();
    for (auto &&[entity, planes, orientation, thickness, dimensions, panel]: width_view) {
        planes.length.max_x = round(dimensions.width * 100000) / 100000;
        planes.length.max_y = round(dimensions.height * 100000) / 100000;
        planes.length.min_x = round((planes.length.max_x - thickness.control->value()) * 100000) / 100000;
        planes.length.min_y = 0;
        PLOG_DEBUG << panel.name << " length plane: (" << planes.length.min_x << ", " << planes.length.min_y << ") to (" << planes.length.max_x << ", "
                   << planes.length.max_y << ")";
    }

    auto height_view = registry.view<DialogPanelPlanes, HeightOrientation, DialogPanelThickness, PanelDimensions, DialogPanel>().proxy();
    for (auto &&[entity, planes, orientation, thickness, dimensions, panel]: height_view) {
        planes.length.max_x = round(dimensions.width * 100000) / 100000;
        planes.length.max_y = round(dimensions.height * 100000) / 100000;
        planes.length.min_x = 0;
        planes.length.min_y = round((planes.length.max_y - thickness.control->value()) * 100000) / 100000;
        PLOG_DEBUG << panel.name << " length plane: (" << planes.length.min_x << ", " << planes.length.min_y << ") to (" << planes.length.max_x << ", "
                   << planes.length.max_y << ")";
    }
}
