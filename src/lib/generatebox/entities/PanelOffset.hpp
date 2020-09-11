//
// Created by Hobbyist Maker on 8/19/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PANELOFFSET_HPP
#define SILVANUSPRO_PANELOFFSET_HPP

#include <Core/UserInterface/FloatSpinnerCommandInput.h>
#include <Fusion/Features/DistanceExtentDefinition.h>
#include "entities/Dimensions.hpp"

namespace silvanus::generatebox::entities {

    struct PanelOffset {
        double value;
        std::string expression;
    };

    struct PanelOffsetParam {
        std::string expression;
    };

    struct PanelOffsetInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct PanelOffsetDimension {
        adsk::core::Ptr<adsk::fusion::ModelParameter> dimension;
    };

}

#endif //SILVANUSPRO_PANELOFFSET_HPP
