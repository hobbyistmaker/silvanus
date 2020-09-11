//
// Created by Hobbyist Maker on 9/12/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PANELMAX_HPP
#define SILVANUSPRO_PANELMAX_HPP

#include <Core/UserInterface/FloatSpinnerCommandInput.h>

namespace silvanus::generatebox::entities {

    struct PanelMaxInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct PanelMaxHeightInput : public PanelMaxInput {};
    struct PanelMaxLengthInput : public PanelMaxInput {};
    struct PanelMaxWidthInput : public PanelMaxInput {};

    struct PanelMaxInsetInput : public PanelMaxInput {};

    struct PanelMaximums {
        double length;
        double width;
        double height;
    };

    struct PanelMaximumExpressions {
        std::string length;
        std::string width;
        std::string height;
    };

}

#endif //SILVANUSPRO_PANELMAX_HPP
