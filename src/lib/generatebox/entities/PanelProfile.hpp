//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_PANELPROFILE_HPP
#define SILVANUSPRO_PANELPROFILE_HPP

#include "Dimensions.hpp"

#include <Fusion/Sketch/SketchLinearDimension.h>

namespace silvanus::generatebox::entities {

    struct PanelProfileDimension {
        double value;
        std::string expression;
    };

    struct PanelProfile {
        PanelProfileDimension length;
        PanelProfileDimension width;
    };

    struct PanelProfileParams {
        std::string length;
        std::string width;
    };

    struct PanelProfileDimensions {
        adsk::core::Ptr<adsk::fusion::SketchLinearDimension> length;
        adsk::core::Ptr<adsk::fusion::SketchLinearDimension> width;
    };

    struct ComparePanelProfile {
        bool operator()(const entities::PanelProfile& a, const entities::PanelProfile& b) const {
            return (a.length.value < b.length.value) & (a.width.value < b.width.value);
        };
    };

}

#endif //SILVANUSPRO_PANELPROFILE_HPP
