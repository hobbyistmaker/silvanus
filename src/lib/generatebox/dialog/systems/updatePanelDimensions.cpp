//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "DialogSystemManager.hpp"

#include "entities/OrientationTags.hpp"
#include "entities/PanelDimension.hpp"

#include <entt/entt.hpp>

using namespace silvanus::generatebox::entities;

using adsk::core::DefaultModelingOrientations::YUpModelingOrientation;

template<class L, class W, class H>
void updatePanelDimensions(entt::registry& registry, entt::entity entity) {
    auto length = registry.ctx<L>().control;
    auto width  = registry.ctx<W>().control;
    auto height = registry.ctx<H>().control;

    PLOG_DEBUG << "Panel dimensions: " << length->value() << ", " << width->value() << ", " << height->value();
    registry.replace<PanelDimensionInputs>(entity, length, width, height);
}

void updatePanelDimensionInputsImpl(entt::registry& registry) {
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

    updatePanelDimensions<DialogLengthInput, DialogWidthInput, DialogTopThickness>(registry, top);
    updatePanelDimensions<DialogLengthInput, DialogWidthInput, DialogBottomThickness>(registry, bottom);
    updatePanelDimensions<DialogLeftThickness, DialogWidthInput, DialogHeightInput>(registry, left);
    updatePanelDimensions<DialogRightThickness, DialogWidthInput, DialogHeightInput>(registry, right);

    if (orientation.value == YUpModelingOrientation) {
        updatePanelDimensions<DialogLengthInput, DialogFrontThickness, DialogHeightInput>(registry, front);
        updatePanelDimensions<DialogLengthInput, DialogBackThickness, DialogHeightInput>(registry, back);
    } else {
        updatePanelDimensions<DialogLengthInput, DialogFrontThickness, DialogHeightInput>(registry, front);
        updatePanelDimensions<DialogLengthInput, DialogBackThickness, DialogHeightInput>(registry, back);
    }
}

void updatePanelDimensionsImpl(entt::registry& registry) {
    registry.view<PanelDimensions, PanelDimensionInputs>().each([](
        auto &dimensions, auto const &inputs
    ){
        dimensions.length = inputs.length->value();
        dimensions.width = inputs.width->value();
        dimensions.height = inputs.height->value();
    });
}

