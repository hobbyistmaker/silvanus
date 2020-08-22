//
// Created by Hobbyist Maker on 8/2/20.
//

#include "PanelFingerSketch.hpp"

#include "Core/Memory.h"

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::fusion;

PanelFingerSketch::PanelFingerSketch(
    const Ptr<ExtrudeFeature>& extrusion,
    const faceSelector& selector,
    const Ptr<Point3D>& start,
    const Ptr<Point3D>& end,
    const std::string& name
) : FusionSketch(name, selector, extrusion, true), m_start{start}, m_end{end}, m_selector{selector}
{
    auto lines = drawFinger(start, end);
}

adsk::core::Ptr<SketchLineList> PanelFingerSketch::drawFinger(const Ptr<Point3D>& start, const Ptr<Point3D>& end) {
    auto start_point = offsetMinPoint(start);
    auto end_point = offsetPoint3D(start_point, end);
    auto lines = m_sketch->sketchCurves()->sketchLines()->addTwoPointRectangle(start_point, end_point);

    addGeometricConstraints(lines);
    addFaceOriginConstraint(lines, minPoint());
    addDistanceDimension(lines->item(0), lines->item(1));
    addDistanceDimension(minPoint(), lines->item(0)->startSketchPoint(), lines->item(1));
    addExtrusionSideConstraints(lines);

    return lines;
}
