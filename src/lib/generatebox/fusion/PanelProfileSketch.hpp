//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/28/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PANELPROFILESKETCH_HPP
#define SILVANUSPRO_PANELPROFILESKETCH_HPP

#include <tuple>

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include "FusionSketch.hpp"
#include "entities/DialogInputs.hpp"
#include "entities/Dimensions.hpp"
#include "entities/PanelProfile.hpp"

namespace silvanus::generatebox::fusion {

    class PanelProfileSketch : public FusionSketch
    {
            adsk::core::Ptr<adsk::fusion::SketchLineList> m_profile_lines;
            std::function<std::tuple<double, double, double>(double, double, double)> m_transform;
            entities::PanelProfile m_profile;

            adsk::core::Ptr<adsk::fusion::SketchLinearDimension> m_profile_length;
            adsk::core::Ptr<adsk::fusion::SketchLinearDimension> m_profile_width;

            void initialize_profile();
            auto draw_profile() -> adsk::core::Ptr<adsk::fusion::SketchLineList>;
            void addProfileDimensions();

        public:
            PanelProfileSketch(
                    const std::string& name,
                    const adsk::core::Ptr<adsk::fusion::BRepFace>& source,
                    std::function<std::tuple<double, double, double>(double, double, double)>  transform,
                    entities::PanelProfile profile
            );

            PanelProfileSketch(
                        const std::string& name,
                        const adsk::core::Ptr<adsk::fusion::ConstructionPlane>& source,
                        std::function<std::tuple<double, double, double>(double, double, double)>  transform,
                        entities::PanelProfile profile
            );

            [[nodiscard]] adsk::core::Ptr<adsk::fusion::ExtrudeFeature> extrudeProfile(
                    const entities::ExtrusionDistance& distance,
                    const entities::PanelOffset& offset
            ) const;

            auto lengthDimension() -> adsk::core::Ptr<adsk::fusion::SketchLinearDimension> {
                return m_profile_length;
            }
            auto widthDimension() -> adsk::core::Ptr<adsk::fusion::SketchLinearDimension> {
                return m_profile_width;
            }
    };

}


#endif /* silvanuspro_panelprofilesketch_hpp */
