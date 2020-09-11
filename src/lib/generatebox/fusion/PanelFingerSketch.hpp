//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_PANELFINGERSKETCH_HPP
#define SILVANUSPRO_PANELFINGERSKETCH_HPP

#include "FusionSketch.hpp"
#include <Core/Geometry/Point3D.h>
#include <Fusion/Features/ExtrudeFeature.h>
#include <Fusion/BRep/BRepBody.h>
#include <Fusion/BRep/BRepFace.h>
#include <Fusion/Sketch/SketchLineList.h>

#include <functional>
#include <string>

namespace silvanus::generatebox::fusion {

    class PanelFingerSketch : public FusionSketch {

        faceSelector m_selector;
        adsk::core::Ptr<adsk::core::Point3D> m_start;
        adsk::core::Ptr<adsk::core::Point3D> m_end;

        adsk::core::Ptr<adsk::fusion::SketchLinearDimension> m_finger_length;
        adsk::core::Ptr<adsk::fusion::SketchLinearDimension> m_origin_offset;

        adsk::core::Ptr<adsk::fusion::SketchLineList> drawFinger(
            const adsk::core::Ptr<adsk::core::Point3D>& offset,
            const adsk::core::Ptr<adsk::core::Point3D>& end
        );

    public:
        PanelFingerSketch(
            const adsk::core::Ptr<adsk::fusion::ExtrudeFeature>& extrusion,
            const faceSelector& selector,
            const adsk::core::Ptr<adsk::core::Point3D>& start,
            const adsk::core::Ptr<adsk::core::Point3D>& end,
            const std::string& name
        );

        auto fingerLength() const -> adsk::core::Ptr<adsk::fusion::ModelParameter> {
            return m_finger_length->parameter();
        }

        auto originOffset() const -> adsk::core::Ptr<adsk::fusion::ModelParameter> {
            return m_origin_offset->parameter();
        }
    };

}

#endif //SILVANUSPRO_PANELFINGERSKETCH_HPP
