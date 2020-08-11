//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_KERFINPUT_HPP
#define SILVANUSPRO_KERFINPUT_HPP

#include "FloatSpinnerCommandInput.h"

namespace silvanus::generatebox::entities {
    struct KerfInput
    {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };
}

#endif //SILVANUSPRO_KERFINPUT_HPP
