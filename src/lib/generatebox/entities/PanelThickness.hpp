//
// Created by Hobbyist Maker on 9/12/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PANELTHICKNESS_HPP
#define SILVANUSPRO_PANELTHICKNESS_HPP

namespace silvanus::generatebox::entities {

    struct PanelThickness {
        double value;
    };

    struct PanelThicknessInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct PanelThicknessActive {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

}

#endif //SILVANUSPRO_PANELTHICKNESS_HPP
