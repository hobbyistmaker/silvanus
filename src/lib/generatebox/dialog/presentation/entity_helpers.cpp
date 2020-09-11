//
// Created by Hobbyist Maker on 9/12/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entity_helpers.hpp"
#include "entities/EntitiesAll.hpp"

#include <Core/UserInterface/CommandInput.h>

#include <entt/entt.hpp>
#include <plog/Log.h>

using namespace adsk::core;
using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox;

auto silvanus::generatebox::makeDialogPanelEntity(entt::registry& registry, Position position, std::string name, int priority, AxisFlag orientation) -> entt::entity {
    auto entity = registry.create();
    registry.emplace<PanelPlanes>(entity);
    registry.emplace<PanelPlanesParams>(entity);
    registry.emplace<PanelDimensions>(entity);
    registry.emplace<PanelEnabled>(entity);
    registry.emplace<PanelThickness>(entity);

    registry.emplace<Panel>(entity, name, priority, orientation);
    registry.emplace<PanelPosition>(entity, position);
    return entity;
}

auto silvanus::generatebox::makePanelEntity(entt::registry& registry) -> entt::entity {
    auto entity = registry.create();
    registry.emplace<ExtrusionDistance>(entity);
    registry.emplace<ExtrusionDistanceParam>(entity);
    registry.emplace<JointProfileParams>(entity);
    registry.emplace<PanelOffset>(entity);
    registry.emplace<PanelOffsetParam>(entity);
    registry.emplace<PanelProfile>(entity);
    registry.emplace<PanelProfileParams>(entity);
    return entity;
}

auto silvanus::generatebox::makeJointEntity(entt::registry& registry) -> entt::entity {
    auto entity = registry.create();
    registry.emplace<ExtrusionDistance>(entity);
    registry.emplace<FingerPattern>(entity);
    registry.emplace<FingerWidth>(entity);
    registry.emplace<JointPattern>(entity);
    return entity;
}

void silvanus::generatebox::maxWidthPanel(entt::registry &registry, entt::entity entity) {
    auto length = registry.ctx<DialogLengthInput>().control;
    auto width = registry.ctx<DialogWidthInput>().control;
    auto height = registry.ctx<DialogHeightInput>().control;

    PLOG_DEBUG << "Adding max width " << width->value() << " to max width panel.";
    registry.emplace<MaxHeightParam>(entity, "height");
    registry.emplace<MaxLengthParam>(entity, "length");
    registry.emplace<MaxWidthParam>(entity, "width");
    registry.emplace<PanelMaxHeightInput>(entity, height);
    registry.emplace<PanelMaxWidthInput>(entity, width);
    registry.emplace<PanelMaxLengthInput>(entity, length);
    registry.emplace<PanelMaxPoint>(entity);
    registry.emplace<PanelMaxParam>(entity);
    registry.emplace<PanelMinPoint>(entity);
    registry.emplace<PanelMinParam>(entity);
    registry.emplace<WidthOrientation>(entity);
    registry.emplace<PanelOrientation>(entity, AxisFlag::Width);
    registry.emplace<OutsidePanel>(entity);
    registry.emplace<PanelAxis>(entity, 0, 1, 0);
}

void silvanus::generatebox::minWidthPanel(entt::registry &registry, entt::entity entity) {
    auto length = registry.ctx<DialogLengthInput>().control;
    auto width = registry.ctx<DialogWidthInput>().control;
    auto height = registry.ctx<DialogHeightInput>().control;

    PLOG_DEBUG << "Configuring min height panel.";
    registry.emplace<PanelMaxHeightInput>(entity, height);
    registry.emplace<PanelMaxLengthInput>(entity, length);
    registry.emplace<MaxHeightParam>(entity, "height");
    registry.emplace<MaxLengthParam>(entity, "length");
    registry.emplace<PanelMaxPoint>(entity);
    registry.emplace<PanelMaxParam>(entity);
    registry.emplace<PanelMinPoint>(entity);
    registry.emplace<PanelMinParam>(entity);
    registry.emplace<WidthOrientation>(entity);
    registry.emplace<PanelOrientation>(entity, AxisFlag::Width);
    registry.emplace<OutsidePanel>(entity);
    registry.emplace<PanelAxis>(entity, 0, 1, 0);
}

void silvanus::generatebox::maxLengthPanel(entt::registry &registry, entt::entity entity) {
    auto length = registry.ctx<DialogLengthInput>().control;
    auto width = registry.ctx<DialogWidthInput>().control;
    auto height = registry.ctx<DialogHeightInput>().control;

    PLOG_DEBUG << "Adding max length " << length->value() << " to right panel.";
    registry.emplace<PanelMaxHeightInput>(entity, height);
    registry.emplace<PanelMaxWidthInput>(entity, width);
    registry.emplace<PanelMaxLengthInput>(entity, length);
    registry.emplace<MaxHeightParam>(entity, "height");
    registry.emplace<MaxLengthParam>(entity, "length");
    registry.emplace<MaxWidthParam>(entity, "width");
    registry.emplace<PanelMaxPoint>(entity);
    registry.emplace<PanelMaxParam>(entity);
    registry.emplace<PanelMinPoint>(entity);
    registry.emplace<PanelMinParam>(entity);
    registry.emplace<LengthOrientation>(entity);
    registry.emplace<PanelOrientation>(entity, AxisFlag::Length);
    registry.emplace<OutsidePanel>(entity);
    registry.emplace<PanelAxis>(entity, 1, 0, 0);
}

void silvanus::generatebox::minLengthPanel(entt::registry &registry, entt::entity entity) {
    auto length = registry.ctx<DialogLengthInput>().control;
    auto width = registry.ctx<DialogWidthInput>().control;
    auto height = registry.ctx<DialogHeightInput>().control;

    PLOG_DEBUG << "Configuring left panel.";
    registry.emplace<PanelMaxHeightInput>(entity, height);
    registry.emplace<PanelMaxWidthInput>(entity, width);
    registry.emplace<MaxHeightParam>(entity, "height");
    registry.emplace<MaxWidthParam>(entity, "width");
    registry.emplace<PanelMaxPoint>(entity);
    registry.emplace<PanelMaxParam>(entity);
    registry.emplace<PanelMinPoint>(entity);
    registry.emplace<PanelMinParam>(entity);
    registry.emplace<LengthOrientation>(entity);
    registry.emplace<PanelOrientation>(entity, AxisFlag::Length);
    registry.emplace<OutsidePanel>(entity);
    registry.emplace<PanelAxis>(entity, 1, 0, 0);
}

void silvanus::generatebox::maxHeightPanel(entt::registry &registry, entt::entity entity) {
    auto length = registry.ctx<DialogLengthInput>().control;
    auto width = registry.ctx<DialogWidthInput>().control;
    auto height = registry.ctx<DialogHeightInput>().control;

    PLOG_DEBUG << "Adding max height " << height->value() << " to top panel.";
    registry.emplace<PanelMaxHeightInput>(entity, height);
    registry.emplace<PanelMaxWidthInput>(entity, width);
    registry.emplace<PanelMaxLengthInput>(entity, length);
    registry.emplace<MaxHeightParam>(entity, "height");
    registry.emplace<MaxLengthParam>(entity, "length");
    registry.emplace<MaxWidthParam>(entity, "width");
    registry.emplace<PanelMaxPoint>(entity);
    registry.emplace<PanelMaxParam>(entity);
    registry.emplace<PanelMinPoint>(entity);
    registry.emplace<PanelMinParam>(entity);
    registry.emplace<HeightOrientation>(entity);
    registry.emplace<PanelOrientation>(entity, AxisFlag::Height);
    registry.emplace<OutsidePanel>(entity);
    registry.emplace<PanelAxis>(entity, 0, 0, 1);
}

void silvanus::generatebox::minHeightPanel(entt::registry &registry, entt::entity entity) {
    auto length = registry.ctx<DialogLengthInput>().control;
    auto width = registry.ctx<DialogWidthInput>().control;

    PLOG_DEBUG << "Configuring bottom panel.";
    registry.emplace<PanelMaxLengthInput>(entity, length);
    registry.emplace<PanelMaxWidthInput>(entity, width);
    registry.emplace<MaxLengthParam>(entity, "length");
    registry.emplace<MaxWidthParam>(entity, "width");
    registry.emplace<PanelMaxPoint>(entity);
    registry.emplace<PanelMaxParam>(entity);
    registry.emplace<PanelMinPoint>(entity);
    registry.emplace<PanelMinParam>(entity);
    registry.emplace<HeightOrientation>(entity);
    registry.emplace<PanelOrientation>(entity, AxisFlag::Height);
    registry.emplace<OutsidePanel>(entity);
    registry.emplace<PanelAxis>(entity, 0, 0, 1);
}
