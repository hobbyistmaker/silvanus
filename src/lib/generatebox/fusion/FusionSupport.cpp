//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/29/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "FusionSupport.hpp"
#include "entities/Dimensions.hpp"

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::fusion;

adsk::core::Ptr<ExtrudeFeatureInput> silvanus::generatebox::fusion::createSimpleExtrusion(
        const adsk::core::Ptr<adsk::fusion::Sketch>& sketch,
        const adsk::core::Ptr<adsk::fusion::Profile>& profile,
        const ExtrusionDistance extrusion_distance,
        const PanelOffset panel_offset
) {
    auto distance = ValueInput::createByReal(extrusion_distance.value);
    auto offset = ValueInput::createByReal(panel_offset.value);

    auto extent = DistanceExtentDefinition::create(distance);
    auto start = FromEntityStartDefinition::create(sketch->referencePlane(), offset);

    auto input = sketch->parentComponent()->features()->extrudeFeatures()->createInput(profile, NewBodyFeatureOperation);
    input->setOneSideExtent(extent, PositiveExtentDirection);
    input->startExtent(start);
    return input;
}

adsk::core::Ptr<ExtrudeFeatureInput> silvanus::generatebox::fusion::createSimpleExtrusion(
        const adsk::core::Ptr<BRepFace>& face,
        const ExtrusionDistance extrusion_distance,
        const PanelOffset panel_offset
) {
    auto distance = ValueInput::createByReal(-extrusion_distance.value);
    auto offset = ValueInput::createByReal(-panel_offset.value);

    auto extent = DistanceExtentDefinition::create(distance);
    auto start = FromEntityStartDefinition::create(face, offset);

    auto input = face->body()->parentComponent()->features()->extrudeFeatures()->createInput(face, NewBodyFeatureOperation);
    input->setOneSideExtent(extent, PositiveExtentDirection);
    input->startExtent(start);
    return input;
}

adsk::core::Ptr<ExtrudeFeatureInput> silvanus::generatebox::fusion::createSimpleExtrusion(
    const adsk::core::Ptr<BRepFace>& face,
    const ExtrusionDistance extrusion_distance,
    const PanelOffset panel_offset,
    const PanelOffset start_offset
) {
    auto distance = ValueInput::createByReal(-extrusion_distance.value);
    auto offset = ValueInput::createByReal(-panel_offset.value + start_offset.value);

    auto extent = DistanceExtentDefinition::create(distance);
    auto start = FromEntityStartDefinition::create(face, offset);

    auto input = face->body()->parentComponent()->features()->extrudeFeatures()->createInput(face, NewBodyFeatureOperation);
    input->setOneSideExtent(extent, PositiveExtentDirection);
    input->startExtent(start);
    return input;
}

adsk::core::Ptr<ExtrudeFeature> silvanus::generatebox::fusion::cutSimpleExtrusion(
    const adsk::core::Ptr<Sketch>& sketch,
    const adsk::core::Ptr<Profile>& profile,
    const double extrusion_distance,
    const double panel_offset,
    adsk::core::Ptr<BRepBody>& body
) {
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
