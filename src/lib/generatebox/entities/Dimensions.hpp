//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_DIMENSIONS_HPP
#define SILVANUSPRO_DIMENSIONS_HPP

#include "DialogInputs.hpp"
#include <Core/CoreAll.h>

namespace silvanus::generatebox::entities {

    struct DimensionsInputs {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> length_control;
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> width_control;
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> height_control;
    };

    struct Dimensions {
        double length = 0;
        double width = 0;
        double height = 0;
        double thickness = 0;
    };

    struct Dimension {
        double value = 0;
    };
    struct ExtrusionDistance : public Dimension {};

    struct PanelOffset : public Dimension {};
    struct JointPanelOffset : public Dimension {};

    struct CompareDimension {
        bool operator()(const entities::Dimension &a, const entities::Dimension &b) const {
            return (a.value < b.value);
        };
    };
}

#endif //SILVANUSPRO_DIMENSIONS_HPP
