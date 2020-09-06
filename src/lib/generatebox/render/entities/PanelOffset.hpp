//
// Created by Hobbyist Maker on 8/19/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PANELOFFSET_HPP
#define SILVANUSPRO_PANELOFFSET_HPP

#include <Core/CoreAll.h>
#include "entities/Dimensions.hpp"

namespace silvanus::generatebox::entities {

    struct PanelOffset : public Dimension {};

    struct PanelOffsetInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

}

#endif //SILVANUSPRO_PANELOFFSET_HPP
