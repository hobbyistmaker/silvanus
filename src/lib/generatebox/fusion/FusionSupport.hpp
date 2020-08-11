//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/29/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_FUSIONSUPPORT_HPP
#define SILVANUSPRO_FUSIONSUPPORT_HPP

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include "Dimensions.hpp"
#include "entities/AxisFlag.hpp"

#include <functional>
#include <map>
#include <unordered_map>

namespace silvanus::generatebox::fusion {

    using axis_plane_map = std::map<entities::AxisFlag, adsk::core::Ptr<adsk::fusion::ConstructionPlane>>;
    using orientation_plane_map = std::unordered_map<adsk::core::DefaultModelingOrientations, axis_plane_map>;
    using axis_transform_map = std::map<entities::AxisFlag, std::function<std::tuple<double, double, double>(double, double, double)>>;
    using orientation_transform_map = std::unordered_map<adsk::core::DefaultModelingOrientations, axis_transform_map>;
    using axisFaceSelector = std::map<entities::AxisFlag, std::function<adsk::core::Ptr<adsk::fusion::BRepFace>(adsk::core::Ptr<adsk::fusion::BRepBody>)>>;
    using orientationAxisSelector = std::map<adsk::core::DefaultModelingOrientations, axisFaceSelector>;

    adsk::core::Ptr<adsk::fusion::ExtrudeFeatureInput> createSimpleExtrusion(
        const adsk::core::Ptr<adsk::fusion::Sketch>& sketch,
        const adsk::core::Ptr<adsk::fusion::Profile>& profile,
        entities::ExtrusionDistance distance,
        entities::PanelOffset offset
    );

    adsk::core::Ptr<adsk::fusion::ExtrudeFeatureInput> createSimpleExtrusion(
        const adsk::core::Ptr<adsk::fusion::BRepFace>& face,
        entities::ExtrusionDistance distance,
        entities::PanelOffset offset
    );

    adsk::core::Ptr<adsk::fusion::ExtrudeFeatureInput> createSimpleExtrusion(
        const adsk::core::Ptr<adsk::fusion::BRepFace>& face,
        entities::ExtrusionDistance distance,
        entities::PanelOffset offset,
        entities::PanelOffset start
    );

    adsk::core::Ptr<adsk::fusion::ExtrudeFeature> cutSimpleExtrusion(
        const adsk::core::Ptr<adsk::fusion::Sketch>& sketch,
        const adsk::core::Ptr<adsk::fusion::Profile>& profile,
        double distance,
        double offset,
        adsk::core::Ptr<adsk::fusion::BRepBody>& body
    );
}

#endif /* silvanuspro_fusionsupport_hpp */
