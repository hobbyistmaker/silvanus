//
// Created by Hobbyist Maker on 8/21/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PANELDIMENSION_HPP
#define SILVANUSPRO_PANELDIMENSION_HPP

#include <Core/CoreAll.h>

namespace silvanus::generatebox::entities {

    struct PanelDimensions {
        double length;
        double width;
        double height;
    };

    struct PanelDimensionInputs {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> length;
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> width;
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> height;
    };

}

#endif //SILVANUSPRO_PANELDIMENSION_HPP
