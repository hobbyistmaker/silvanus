//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_THICKNESSINPUT_HPP
#define SILVANUSPRO_THICKNESSINPUT_HPP

#include "FloatSpinnerCommandInput.h"

namespace silvanus::generatebox::entities {
    struct ThicknessInput
    {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };
}

#endif //SILVANUSPRO_THICKNESSINPUT_HPP
