//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_ENABLEINPUT_HPP
#define SILVANUSPRO_ENABLEINPUT_HPP

#include <BoolValueCommandInput.h>
#include <FloatSpinnerCommandInput.h>

namespace silvanus::generatebox::entities {
    struct EnableInput
    {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> control;
    };
}

#endif //SILVANUSPRO_ENABLEINPUT_HPP
