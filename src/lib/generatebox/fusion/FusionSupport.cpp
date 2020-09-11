//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/29/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "FusionSupport.hpp"
#include "entities/Dimensions.hpp"
#include "entities/JointThickness.hpp"
#include "entities/PanelOffset.hpp"

#include <plog/Log.h>
#include <fmt/format.h>

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::fusion;

auto silvanus::generatebox::fusion::createSimpleExtrusion(
        const adsk::core::Ptr<adsk::fusion::Sketch>& sketch,
        const adsk::core::Ptr<adsk::fusion::Profile>& profile,
        const ExtrusionDistance& extrusion_distance,
        const PanelOffset& panel_offset
) -> adsk::core::Ptr<ExtrudeFeatureInput> {
    PLOG_DEBUG << "Creating simple extrusion with distance " << extrusion_distance.expression << " and offset " << panel_offset.expression;
    auto has_distance_expression = extrusion_distance.expression.length() > 0;
    auto has_offset_expression = panel_offset.expression.length() > 0;
    auto distance = has_distance_expression ? ValueInput::createByString(extrusion_distance.expression) : ValueInput::createByReal(extrusion_distance.value);
    auto offset = has_offset_expression ? ValueInput::createByString(panel_offset.expression) : ValueInput::createByReal(panel_offset.value);

    auto extent = DistanceExtentDefinition::create(distance);
    auto start = FromEntityStartDefinition::create(sketch->referencePlane(), offset);

    auto input = sketch->parentComponent()->features()->extrudeFeatures()->createInput(profile, NewBodyFeatureOperation);
    input->setOneSideExtent(extent, PositiveExtentDirection);
    input->startExtent(start);
    return input;
}

auto silvanus::generatebox::fusion::createSimpleExtrusion(
        const adsk::core::Ptr<BRepFace>& face,
        const ExtrusionDistance& extrusion_distance,
        const PanelOffset& panel_offset
) -> adsk::core::Ptr<ExtrudeFeatureInput> {
    auto has_distance_expression = extrusion_distance.expression.length() > 0;
    auto has_offset_expression = panel_offset.expression.length() > 0;

    auto distance_expression = std::string{"-("}.append(extrusion_distance.expression).append(")");
    auto offset_expression = std::string{"-("}.append(panel_offset.expression).append(")");

    auto distance = has_distance_expression ? ValueInput::createByString(distance_expression) : ValueInput::createByReal(-extrusion_distance.value);
    auto offset = has_offset_expression ? ValueInput::createByString(offset_expression) : ValueInput::createByReal(-panel_offset.value);

    auto extent = DistanceExtentDefinition::create(distance);
    auto start = FromEntityStartDefinition::create(face, offset);

    auto input = face->body()->parentComponent()->features()->extrudeFeatures()->createInput(face, NewBodyFeatureOperation);
    input->setOneSideExtent(extent, PositiveExtentDirection);
    input->startExtent(start);
    return input;
}

auto silvanus::generatebox::fusion::createSimpleExtrusion(
    const adsk::core::Ptr<BRepFace>& face,
    const ExtrusionDistance& extrusion_distance,
    const PanelOffset& panel_offset,
    const PanelOffset& start_offset
) -> adsk::core::Ptr<ExtrudeFeatureInput> {
    PLOG_DEBUG << "Starting simple extrusion with offset expression: " << panel_offset.expression;
    auto has_distance_expression = extrusion_distance.expression.length() > 0;
    auto has_offset_expression = panel_offset.expression.length() > 0;
    auto has_start_expression = start_offset.expression.length() > 0;

    auto distance_expression = has_distance_expression ? fmt::format("-({})", extrusion_distance.expression) : "";
    auto modified_start_expression = has_start_expression ? fmt::format(" + ({})", start_offset.expression) : "";
    auto offset_expression = has_offset_expression ? fmt::format("((-({0})){1})", panel_offset.expression, modified_start_expression) : "";
    PLOG_DEBUG << "Creating simple extrusion with offset expression: " << offset_expression;

    auto distance = has_distance_expression ? ValueInput::createByString(distance_expression) : ValueInput::createByReal(-extrusion_distance.value);
    auto offset = has_offset_expression ? ValueInput::createByString(offset_expression) : ValueInput::createByReal(-panel_offset.value + start_offset.value);

    auto extent = DistanceExtentDefinition::create(distance);
    auto start = FromEntityStartDefinition::create(face, offset);

    auto input = face->body()->parentComponent()->features()->extrudeFeatures()->createInput(face, NewBodyFeatureOperation);
    input->setOneSideExtent(extent, PositiveExtentDirection);
    input->startExtent(start);
    return input;
}

auto silvanus::generatebox::fusion::cutSimpleExtrusion(
    const adsk::core::Ptr<Sketch>& sketch,
    const adsk::core::Ptr<Profile>& profile,
    const double extrusion_distance,
    const double panel_offset,
    const adsk::core::Ptr<BRepBody>& body
) -> adsk::core::Ptr<ExtrudeFeature> {
    auto distance = ValueInput::createByReal(extrusion_distance);
    auto offset = ValueInput::createByReal(panel_offset * -1);

    auto extent = DistanceExtentDefinition::create(distance);
    auto start = FromEntityStartDefinition::create(sketch->referencePlane(), offset);

    auto input = sketch->parentComponent()->features()->extrudeFeatures()->createInput(profile, CutFeatureOperation);
    input->setOneSideExtent(extent, NegativeExtentDirection);
    input->startExtent(start);
    input->participantBodies({body});

    return sketch->parentComponent()->features()->extrudeFeatures()->add(input);
}

auto silvanus::generatebox::fusion::cutSimpleExtrusion(
    const adsk::core::Ptr<Sketch>& sketch,
    const adsk::core::Ptr<Profile>& profile,
    const JointThickness& distance,
    const JointPanelOffset& offset,
    const adsk::core::Ptr<BRepBody>& body
) -> adsk::core::Ptr<ExtrudeFeature> {
    auto distance_input = ValueInput::createByReal(distance.value);
    auto offset_input = ValueInput::createByReal(offset.value * -1);
    auto extent = DistanceExtentDefinition::create(distance_input);
    auto start = FromEntityStartDefinition::create(sketch->referencePlane(), offset_input);

    auto input = sketch->parentComponent()->features()->extrudeFeatures()->createInput(profile, CutFeatureOperation);
    input->setOneSideExtent(extent, NegativeExtentDirection);
    input->startExtent(start);
    input->participantBodies({body});

    return sketch->parentComponent()->features()->extrudeFeatures()->add(input);
}
