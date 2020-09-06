//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_MAXOFFSETINPUT_HPP
#define SILVANUSPRO_MAXOFFSETINPUT_HPP

#include "lib/generatebox/dialog/entities/DialogInputs.hpp"

namespace silvanus::generatebox::entities {
    struct MaxOffsetInput
    {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };
}

#endif //SILVANUSPRO_MAXOFFSETINPUT_HPP
