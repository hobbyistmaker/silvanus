//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/28/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_FUSIONSKETCH_HPP
#define SILVANUSPRO_FUSIONSKETCH_HPP

#include <functional>
#include <set>
#include <tuple>

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include "entities/Dimensions.hpp"
#include "entities/JointThickness.hpp"

using sketchPointTuple = std::tuple<adsk::core::Ptr<adsk::fusion::SketchPoint>, adsk::core::Ptr<adsk::fusion::SketchPoint>>;

namespace silvanus::generatebox::fusion {

    using faceSelector = std::function<adsk::core::Ptr<adsk::fusion::BRepFace>(adsk::core::Ptr<adsk::fusion::BRepBody>)>;

    struct AscendingSketchGeometry {
        bool operator()(const adsk::core::Ptr<adsk::fusion::SketchPoint> lhs, const adsk::core::Ptr<adsk::fusion::SketchPoint> rhs) const {
            auto lhs_g = lhs->geometry();
            auto rhs_g = rhs->geometry();

            return (lhs_g->x() < rhs_g->x()) || (lhs_g->y() < rhs_g->y()) || (lhs_g->z() < rhs_g->z());
        }
    };

    struct AscendingWorldGeometry {
        bool operator()(const adsk::core::Ptr<adsk::fusion::SketchPoint> lhs, const adsk::core::Ptr<adsk::fusion::SketchPoint> rhs) const {
            auto lhs_g = lhs->worldGeometry();
            auto rhs_g = rhs->worldGeometry();

            return (lhs_g->x() < rhs_g->x()) || (lhs_g->y() < rhs_g->y()) || (lhs_g->z() < rhs_g->z());
        }
    };

    class FusionSketch
    {
            void initialize_sketch(const std::string& name, bool construction);

        protected:

            std::string m_name;
            adsk::core::Ptr<adsk::fusion::Sketch> m_sketch;
            std::vector<adsk::core::Ptr<adsk::fusion::SketchPoint>> m_default_world_points;
            std::vector<adsk::core::Ptr<adsk::fusion::SketchPoint>> m_default_sketch_points;

            bool positiveSketch();
            bool isVerticalSketch();

            void addGeometricConstraints(const adsk::core::Ptr<adsk::fusion::SketchLineList>& lines);
            void addFaceOriginConstraint(
                    const adsk::core::Ptr<adsk::fusion::SketchLineList>& lines,
                    const adsk::core::Ptr<adsk::fusion::SketchPoint>& point
            );
            void addOriginConstraint(const adsk::core::Ptr<adsk::fusion::SketchLineList>& lines);
            void addDistanceDimension(const adsk::core::Ptr<adsk::fusion::SketchLine>& line);
            void addDistanceDimension(
                const adsk::core::Ptr<adsk::fusion::SketchLine>& line,
                const adsk::core::Ptr<adsk::fusion::SketchLine>& normal
            );
            void addDistanceDimension(const adsk::core::Ptr<adsk::fusion::SketchPoint>& lhs, const adsk::core::Ptr<adsk::fusion::SketchPoint>& rhs);
            void addDistanceDimension(
                const adsk::core::Ptr<adsk::fusion::SketchPoint>& lhs,
                const adsk::core::Ptr<adsk::fusion::SketchPoint>& rhs,
                const adsk::core::Ptr<adsk::fusion::SketchLine>& normal
            );
            void addExtrusionSideConstraints(const adsk::core::Ptr<adsk::fusion::SketchLineList>& lines);

            adsk::core::Ptr<adsk::fusion::SketchPoint> minPoint();
            adsk::core::Ptr<adsk::fusion::SketchPoint> maxPoint();

            adsk::core::Ptr<adsk::core::Point3D> offsetSketchPoint(
                const adsk::core::Ptr<adsk::fusion::SketchPoint>& start,
                const adsk::core::Ptr<adsk::core::Point3D>& end
            );
            adsk::core::Ptr<adsk::core::Point3D> offsetPoint3D(
                const adsk::core::Ptr<adsk::core::Point3D>& point,
                const adsk::core::Ptr<adsk::core::Point3D>& end
            );
            adsk::core::Ptr<adsk::core::Point3D> offsetMinPoint(
                const adsk::core::Ptr<adsk::core::Point3D>& end
            );

        public:
            FusionSketch(const std::string& name, const adsk::core::Ptr<adsk::fusion::BRepFace>& source, bool construction);
            FusionSketch(const std::string& name, const adsk::core::Ptr<adsk::fusion::ConstructionPlane>& source, bool construction);
            FusionSketch(const std::string& name, const faceSelector& selector, const adsk::core::Ptr<adsk::fusion::ExtrudeFeature>& body, bool construction);
            ~FusionSketch();

            [[nodiscard]] adsk::core::Ptr<adsk::fusion::Profiles> profiles() const { return m_sketch->profiles(); };
            [[nodiscard]] adsk::core::Ptr<adsk::fusion::ExtrudeFeature> cut(
                const entities::PanelOffset& offset,
                const entities::ExtrusionDistance& depth,
                adsk::core::Ptr<adsk::fusion::BRepBody> body
            ) const;
            [[nodiscard]] adsk::core::Ptr<adsk::fusion::Component> parentComponent() const;
    };

}


#endif /* silvanuspro_fusionsketch_hpp */
