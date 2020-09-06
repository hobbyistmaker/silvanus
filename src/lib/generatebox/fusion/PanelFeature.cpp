//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/29/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "PanelFeature.hpp"

#include "FusionSupport.hpp"
#include "entities/Dimensions.hpp"

using namespace adsk::core;
using namespace adsk::fusion;
using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::fusion;

auto PanelFeature::extrudeCopy(const ExtrusionDistance& distance, const PanelOffset& offset) const -> Ptr<ExtrudeFeature> {
    auto face = m_panel->startFaces()->item(0);
    auto const& input = silvanus::generatebox::fusion::createSimpleExtrusion(face, distance, offset);

    return face->body()->parentComponent()->features()->extrudeFeatures()->add(input);
}

auto PanelFeature::extrudeCopy(const ExtrusionDistance& distance, const PanelOffset& offset, const PanelOffset& start) const -> Ptr<ExtrudeFeature> {
    auto face = m_panel->startFaces()->item(0);
    auto const& input = silvanus::generatebox::fusion::createSimpleExtrusion(face, distance, offset, start);

    return face->body()->parentComponent()->features()->extrudeFeatures()->add(input);
}
