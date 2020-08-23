//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_TOGGLEABLETHICKNESSINPUT_HPP
#define SILVANUSPRO_TOGGLEABLETHICKNESSINPUT_HPP

#include <BoolValueCommandInput.h>
#include <FloatSpinnerCommandInput.h>
#include "lib/generatebox/dialog/entities/DialogInputs.hpp"

namespace silvanus::generatebox::entities {
    struct ToggleableThicknessInput
    {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> selector;
        DialogInputs enabled_reference;
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> enabled;
        DialogInputs disabled_reference;
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> disabled;
    };
}

#endif //SILVANUSPRO_TOGGLEABLETHICKNESSINPUT_HPP
