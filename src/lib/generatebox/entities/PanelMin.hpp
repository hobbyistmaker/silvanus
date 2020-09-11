//
// Created by Hobbyist Maker on 9/12/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PANELMIN_HPP
#define SILVANUSPRO_PANELMIN_HPP

#include <Core/UserInterface/FloatSpinnerCommandInput.h>

#include <string>

namespace silvanus::generatebox::entities {

    struct PanelMinInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct PanelMinHeightInput : public PanelMinInput {};
    struct PanelMinLengthInput : public PanelMinInput {};
    struct PanelMinWidthInput : public PanelMinInput {};

    struct PanelMinInsetInput : public PanelMinInput {};

    struct PanelMinimums {
        double length;
        double width;
        double height;
    };

    struct PanelMinimumExpressions {
        std::string length;
        std::string width;
        std::string height;
    };

}

#endif //SILVANUSPRO_PANELMIN_HPP
