//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/28/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "FusionSketch.hpp"

#include "FusionSupport.hpp"

#include <algorithm>
#include <unordered_map>

using std::string;

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::fusion;

FusionSketch::FusionSketch(const string& name, const Ptr<BRepFace>& source, bool construction) : m_name{name} {
    m_sketch = source->body()->parentComponent()->sketches()->add(source);
    initialize_sketch(name, construction);
}

FusionSketch::FusionSketch(const string& name, const Ptr<ConstructionPlane>& source, bool construction) : m_name{name} {
    m_sketch = source->component()->sketches()->add(source);
    initialize_sketch(name, construction);
}

FusionSketch::FusionSketch(const string& name, const faceSelector& selector, const Ptr<ExtrudeFeature>& feature, bool construction) : m_name{name} {
    auto source = selector(feature->bodies()->item(0));
    m_sketch = source->body()->parentComponent()->sketches()->add(source);
    initialize_sketch(name, construction);
}

FusionSketch::~FusionSketch() {
    m_sketch->isComputeDeferred(false);
}

void FusionSketch::initialize_sketch(const string& name, bool construction) {
    m_sketch->name(name);
    m_sketch->isComputeDeferred(true);

    for (auto const& line: m_sketch->sketchCurves()->sketchLines()) {
        line->isConstruction(construction);

        for (auto const& point: {line->startSketchPoint(), line->endSketchPoint()}) {
            m_default_sketch_points.emplace_back(point);
        }

        for (auto const& point: {line->startSketchPoint(), line->endSketchPoint()}) {
            m_default_world_points.emplace_back(point);
        }
    }

    std::sort(m_default_sketch_points.begin(), m_default_sketch_points.end(), AscendingSketchGeometry());
    std::sort(m_default_world_points.begin(), m_default_world_points.end(), AscendingWorldGeometry());
}

void FusionSketch::addExtrusionSideConstraints(const Ptr<SketchLineList> &lines) {

    if (isVerticalSketch()) {
        auto first_sketch_line = m_sketch->sketchCurves()->sketchLines()->item(1);
        auto second_sketch_line = m_sketch->sketchCurves()->sketchLines()->item(3);
        m_sketch->geometricConstraints()->addCollinear(first_sketch_line, lines->item(1));
        m_sketch->geometricConstraints()->addCollinear(second_sketch_line, lines->item(3));
    } else {
        auto first_sketch_line = m_sketch->sketchCurves()->sketchLines()->item(0);
        auto second_sketch_line = m_sketch->sketchCurves()->sketchLines()->item(2);
        m_sketch->geometricConstraints()->addCollinear(first_sketch_line, lines->item(0));
        m_sketch->geometricConstraints()->addCollinear(second_sketch_line, lines->item(2));
    }
}

void FusionSketch::addGeometricConstraints(const Ptr<SketchLineList>& lines) {
    auto constraints = m_sketch->geometricConstraints();

    auto selector = std::unordered_map<bool, std::function<Ptr<HorizontalConstraint>(Ptr<SketchLine>)>>{
            {true, [&](const Ptr<SketchLine>& line){ return constraints->addVertical(line); } },
            {false, [&](const Ptr<SketchLine>& line){ return constraints->addHorizontal(line); } }
    };

    for (auto& line: lines) {
        auto x = line->startSketchPoint()->geometry()->x();
        auto y = line->endSketchPoint()->geometry()->x();
        selector[x == y](line);
    }
}

bool pointsAreEqual(const Ptr<Point3D>& a, const Ptr<Point3D>& b) {
    return (a->x() == b->x()) && (a->y() == b->y()) && (a->z() == b->z());
}

bool FusionSketch::isVerticalSketch() {
    auto first_line = m_sketch->sketchCurves()->sketchLines()->item(0);

    return first_line->startSketchPoint()->geometry()->x() == first_line->endSketchPoint()->geometry()->x();
}

void FusionSketch::addFaceOriginConstraint(const Ptr<SketchLineList>& lines, const Ptr<SketchPoint>& point) {
    auto origin = point->geometry();

    for (auto& line: lines) {
        if (pointsAreEqual(origin, line->startSketchPoint()->geometry())) {
            m_sketch->geometricConstraints()->addCoincident(
                    point, line->startSketchPoint()
            );
            break;
        }
    }
}

void FusionSketch::addOriginConstraint(const Ptr<SketchLineList>& lines) {
    addFaceOriginConstraint(lines, m_sketch->originPoint());
}

adsk::core::Ptr<SketchPoint> FusionSketch::minPoint() {
    return m_default_world_points.front();
}

adsk::core::Ptr<SketchPoint> FusionSketch::maxPoint() {
    return m_default_world_points.back();
}

bool FusionSketch::positiveSketch() {
    auto sketch_min = m_default_sketch_points.front();
    auto world_min = m_default_world_points.front();
    return sketch_min == world_min;
}

void FusionSketch::addDistanceDimension(const Ptr<SketchLine>& line) {
    auto lhs = line->startSketchPoint();
    auto text_point = Point3D::create(lhs->geometry()->x(), lhs->geometry()->y(), lhs->geometry()->z());
    text_point->x(text_point->x() - 1);
    text_point->y(text_point->y() - 1);

    m_sketch->sketchDimensions()->addDistanceDimension(
            line->startSketchPoint(), line->endSketchPoint(), AlignedDimensionOrientation, text_point
    );
}

void FusionSketch::addDistanceDimension(const Ptr<SketchLine>& line, const Ptr<SketchLine>& normal) {
    auto text_x = (normal->startSketchPoint()->geometry()->x() - line->startSketchPoint()->geometry()->x())/2;
    auto text_y = normal->startSketchPoint()->geometry()->y() - normal->endSketchPoint()->geometry()->y();

    auto lhs = line->startSketchPoint();
    auto lhs_x = lhs->geometry()->x();
    auto lhs_y = lhs->geometry()->y();

    auto text_point = Point3D::create(lhs_x + text_x, lhs_y + text_y, lhs->geometry()->z());

    m_sketch->sketchDimensions()->addDistanceDimension(
        line->startSketchPoint(), line->endSketchPoint(), AlignedDimensionOrientation, text_point
    );
}

void FusionSketch::addDistanceDimension(const Ptr<SketchPoint>& lhs, const Ptr<SketchPoint>& rhs) {
    auto text_point = Point3D::create(lhs->geometry()->x(), lhs->geometry()->y(), lhs->geometry()->z());
    text_point->x(text_point->x() - 1);
    text_point->y(text_point->y() - 1);

    m_sketch->sketchDimensions()->addDistanceDimension(
        lhs, rhs, AlignedDimensionOrientation, text_point
    );
}

void FusionSketch::addDistanceDimension(const Ptr<SketchPoint>& lhs, const Ptr<SketchPoint>& rhs, const Ptr<SketchLine>& normal) {
    auto text_x = (rhs->geometry()->x() - lhs->geometry()->x())/2;
    auto text_y = normal->startSketchPoint()->geometry()->y() - normal->endSketchPoint()->geometry()->y();

    auto lhs_x = lhs->geometry()->x();
    auto lhs_y = lhs->geometry()->y();

    auto text_point = Point3D::create(lhs_x + text_x, lhs_y + text_y, lhs->geometry()->z());

    m_sketch->sketchDimensions()->addDistanceDimension(
        lhs, rhs, AlignedDimensionOrientation, text_point
    );
}

Ptr<Point3D> FusionSketch::offsetMinPoint(const Ptr<Point3D>& end) {
    auto min_point = minPoint();
    auto factor_x = positiveSketch() ? 1.0 : -1.0;
    auto start_x = min_point->geometry()->x();
    auto start_y = min_point->geometry()->y();
    auto start_z = min_point->geometry()->z();
    return Point3D::create(start_x + end->x() * factor_x, start_y + end->y(), start_z + end->z());
}

Ptr<Point3D> FusionSketch::offsetSketchPoint(const Ptr<SketchPoint>& start, const Ptr<Point3D>& end) {
    auto factor_x = positiveSketch() ? 1.0 : -1.0;
    auto start_x = start->geometry()->x();
    auto start_y = start->geometry()->y();
    auto start_z = start->geometry()->z();
    return Point3D::create(start_x + end->x() * factor_x, start_y + end->y(), start_z + end->z());
}

Ptr<Point3D> FusionSketch::offsetPoint3D(const Ptr<Point3D>& start, const Ptr<Point3D>& end) {
    auto factor_x = positiveSketch() ? 1.0 : -1.0;
    auto start_x = start->x();
    auto start_y = start->y();
    auto start_z = start->z();
    return Point3D::create(start_x + end->x() * factor_x, start_y + end->y(), start_z + end->z());
}

Ptr<ExtrudeFeature> FusionSketch::cut(const PanelOffset& offset, const ExtrusionDistance& depth, Ptr<BRepBody> body) const {
    return cutSimpleExtrusion(m_sketch, m_sketch->profiles()->item(0), depth.value, offset.value, body);
}

Ptr<Component> FusionSketch::parentComponent() const {
    return m_sketch->parentComponent();
}
