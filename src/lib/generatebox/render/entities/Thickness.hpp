//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_THICKNESS_HPP
#define SILVANUSPRO_THICKNESS_HPP

#include "FloatSpinnerCommandInput.h"

namespace silvanus::generatebox::entities {

    struct Thickness {
        double value;
    };

    struct ThicknessInput
    {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };
}

#endif //SILVANUSPRO_THICKNESS_HPP