//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/29/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "PanelFeature.hpp"

#include "FusionSupport.hpp"
#include "entities/Dimensions.hpp"

#include <plog/Log.h>

using namespace adsk::core;
using namespace adsk::fusion;
using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::fusion;

auto PanelFeature::extrudeCopy(const ExtrusionDistance& distance, const PanelOffset& offset) const -> Ptr<ExtrudeFeature> {
    auto face = m_panel->startFaces()->item(0);
    auto const& input = silvanus::generatebox::fusion::createSimpleExtrusion(face, distance, offset);

    auto feature = face->body()->parentComponent()->features()->extrudeFeatures()->add(input);

    auto has_distance_expression = distance.expression.length() > 0;
    if (has_distance_expression) {
        auto distance_param = Ptr<DistanceExtentDefinition>{feature->extentOne()}->distance();
        auto distance_expression = std::string{"-("}.append(distance.expression).append(")");
        distance_param->expression(distance_expression); // Reassign since Fusion appears to throw away the string expression on creation
    }

    return feature;
}

auto PanelFeature::extrudeCopy(const ExtrusionDistance& distance, const PanelOffset& offset, const PanelOffset& start) const -> Ptr<ExtrudeFeature> {
    PLOG_DEBUG << "Panel copy offset expression: " << offset.expression;
    auto face = m_panel->startFaces()->item(0);
    auto const& input = silvanus::generatebox::fusion::createSimpleExtrusion(face, distance, offset, start);

    auto feature =  face->body()->parentComponent()->features()->extrudeFeatures()->add(input);

    auto has_distance_expression = distance.expression.length() > 0;
    if (has_distance_expression) {
        PLOG_DEBUG << "Offset param for copy: " << Ptr<ModelParameter>{Ptr<FromEntityStartDefinition>{feature->startExtent()}->offset()}->expression();
        auto distance_param = Ptr<DistanceExtentDefinition>{feature->extentOne()}->distance();
        auto distance_expression = std::string{"-("}.append(distance.expression).append(")");
        PLOG_DEBUG << "Using distance expression for copy: " << distance_expression;
        distance_param->expression(distance_expression); // Reassign since Fusion appears to throw away the string expression on creation
    }

    return feature;
}
