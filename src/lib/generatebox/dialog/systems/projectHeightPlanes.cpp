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

void projectHeightPlanesImpl(entt::registry& registry) {
    PLOG_DEBUG << "CreateDialog::projectHeightPlane";

    auto length_view = registry.view<DialogPanelPlanes, LengthOrientation, DialogPanelThickness, PanelDimensions, DialogPanel>().proxy();
    for (auto &&[entity, planes, orientation, thickness, dimensions, panel]: length_view) {
        planes.height.max_x = round(dimensions.length * 100000) / 100000;
        planes.height.max_y = round(dimensions.width * 100000) / 100000;
        planes.height.min_x = round((planes.height.max_x - thickness.control->value()) * 100000) / 100000;
        planes.height.min_y = 0;
        PLOG_DEBUG << panel.name << " height plane: (" << planes.height.min_x << ", " << planes.height.min_y << ") to (" << planes.height.max_x << ", "
                   << planes.height.max_y << ")";
    }

    auto width_view = registry.view<DialogPanelPlanes, WidthOrientation, DialogPanelThickness, PanelDimensions, DialogPanel>().proxy();
    for (auto &&[entity, planes, orientation, thickness, dimensions, panel]: width_view) {
        planes.height.max_x = round(dimensions.length * 100000) / 100000;
        planes.height.max_y = round(dimensions.width * 100000) / 100000;
        planes.height.min_x = 0;
        planes.height.min_y = round((planes.height.max_y - thickness.control->value()) * 100000) / 100000;
        PLOG_DEBUG << panel.name << " height plane: (" << planes.height.min_x << ", " << planes.height.min_y << ") to (" << planes.height.max_x << ", "
                   << planes.height.max_y << ")";
    }

    auto height_view = registry.view<DialogPanelPlanes, HeightOrientation, DialogPanelThickness, PanelDimensions, DialogPanel>().proxy();
    for (auto &&[entity, planes, orientation, thickness, dimensions, panel]: height_view) {
        planes.height.max_x = round(dimensions.length * 100000) / 100000;
        planes.height.max_y = round(dimensions.width * 100000) / 100000;
        planes.height.min_x = 0;
        planes.height.min_y = 0;
        PLOG_DEBUG << panel.name << " height plane: (" << planes.height.min_x << ", " << planes.height.min_y << ") to (" << planes.height.max_x << ", "
                   << planes.height.max_y << ")";
    }
}
