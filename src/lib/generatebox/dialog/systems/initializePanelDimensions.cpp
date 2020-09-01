//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "DialogInputs.hpp"

#include "addPanelDimensions.hpp"
#include "addMaxOffset.hpp"

#include <plog/Log.h>

#include <Core/CoreTypeDefs.h>

using namespace silvanus::generatebox::entities;

using adsk::core::DefaultModelingOrientations::YUpModelingOrientation;

void initializePanelDimensionInputsImpl(entt::registry& registry) {
    auto orientation = registry.ctx<DialogModelingOrientation>();

    auto top    = registry.ctx<DialogTopPanel>().id;
    auto bottom = registry.ctx<DialogBottomPanel>().id;
    auto left   = registry.ctx<DialogLeftPanel>().id;
    auto right  = registry.ctx<DialogRightPanel>().id;
    auto front  = registry.ctx<DialogFrontPanel>().id;
    auto back   = registry.ctx<DialogBackPanel>().id;

    PLOG_DEBUG << "Top Panel ID: " + std::to_string((int) top);
    PLOG_DEBUG << "Bottom Panel ID: " + std::to_string((int) bottom);
    PLOG_DEBUG << "Left Panel ID: " + std::to_string((int) left);
    PLOG_DEBUG << "Right Panel ID: " + std::to_string((int) right);
    PLOG_DEBUG << "Front Panel ID: " + std::to_string((int) front);
    PLOG_DEBUG << "Back Panel ID: " + std::to_string((int) back);

    addPanelDimensions<DialogLengthInput, DialogWidthInput, DialogHeightInput>(registry, top);
    addPanelDimensions<DialogLengthInput, DialogWidthInput, DialogBottomThickness>(registry, bottom);
    addPanelDimensions<DialogLeftThickness, DialogWidthInput, DialogHeightInput>(registry, left);
    addPanelDimensions<DialogLengthInput, DialogWidthInput, DialogHeightInput>(registry, right);

    if (orientation.value == YUpModelingOrientation) {
        addPanelDimensions<DialogLengthInput, DialogWidthInput, DialogHeightInput>(registry, front);
        addPanelDimensions<DialogLengthInput, DialogBackThickness, DialogHeightInput>(registry, back);
    } else {
        addPanelDimensions<DialogLengthInput, DialogFrontThickness, DialogHeightInput>(registry, front);
        addPanelDimensions<DialogLengthInput, DialogWidthInput, DialogHeightInput>(registry, back);
    }

    addMaxOffset<DialogHeightInput>(registry, top);
    addMaxOffset<DialogHeightInput>(registry, bottom);
    addMaxOffset<DialogLengthInput>(registry, left);
    addMaxOffset<DialogLengthInput>(registry, right);
    addMaxOffset<DialogWidthInput>(registry, front);
    addMaxOffset<DialogWidthInput>(registry, back);
}