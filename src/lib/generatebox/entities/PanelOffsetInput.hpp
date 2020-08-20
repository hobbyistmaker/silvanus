//
// Created by Hobbyist Maker on 8/19/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PANELOFFSETINPUT_HPP
#define SILVANUSPRO_PANELOFFSETINPUT_HPP

#include "Core/CoreAll.h"

namespace silvanus::generatebox::entities {

    struct PanelOffsetInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

}

#endif //SILVANUSPRO_PANELOFFSETINPUT_HPP
