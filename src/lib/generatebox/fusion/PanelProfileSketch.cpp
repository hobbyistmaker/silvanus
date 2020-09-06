//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/28/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "PanelProfileSketch.hpp"
#include "FusionSupport.hpp"
#include "entities/Dimensions.hpp"
#include "entities/PanelProfile.hpp"

#include <utility>

using std::string;

using namespace adsk::core;
using namespace adsk::fusion;
using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::fusion;

PanelProfileSketch::PanelProfileSketch(
        const string& name,
        const Ptr<BRepFace>& source,
        std::function<std::tuple<double, double, double>(double, double, double)> transform,
        const PanelProfile& profile
) : FusionSketch{name, source, true}, m_transform{std::move(transform)}, m_profile{profile} {
    initialize_profile();
}

PanelProfileSketch::PanelProfileSketch(
        const string& name,
        const Ptr<ConstructionPlane>& source,
        std::function<std::tuple<double, double, double>(double, double, double)>  transform,
        const PanelProfile& profile
) : FusionSketch{name, source, true}, m_transform{std::move(transform)}, m_profile{profile} {
    initialize_profile();
}

void PanelProfileSketch::initialize_profile() {
    m_profile_lines = draw_profile();

    addGeometricConstraints(m_profile_lines);
    addOriginConstraint(m_profile_lines);
    addProfileDimensions();
}

auto PanelProfileSketch::draw_profile() -> Ptr<SketchLineList> {
    double x, y, z;
    std::tie(x, y, z) = m_transform(m_profile.length.value, m_profile.width.value, 0);

    auto origin = Point3D::create(0, 0, 0);
    auto end_point = Point3D::create(x, y, 0);

    return m_sketch->sketchCurves()->sketchLines()->addTwoPointRectangle(origin, end_point);
}

void PanelProfileSketch::addProfileDimensions() {
    for (int n : {0, 1}) {
        addDistanceDimension(m_profile_lines->item(n));
    }
}

auto PanelProfileSketch::extrudeProfile(const ExtrusionDistance distance, const PanelOffset offset) const -> Ptr<ExtrudeFeature> {
    const auto& input = silvanus::generatebox::fusion::createSimpleExtrusion(m_sketch, m_sketch->profiles()->item(0), distance, offset);

    return m_sketch->parentComponent()->features()->extrudeFeatures()->add(input);
}
