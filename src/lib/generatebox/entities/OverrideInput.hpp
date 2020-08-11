//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_OVERRIDEINPUT_HPP
#define SILVANUSPRO_OVERRIDEINPUT_HPP

#include "BoolValueCommandInput.h"

namespace silvanus::generatebox::entities {
    struct OverrideInput
    {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> control;
    };
}

#endif //SILVANUSPRO_OVERRIDEINPUT_HPP
