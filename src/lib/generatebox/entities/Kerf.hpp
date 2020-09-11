//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_KERF_HPP
#define SILVANUSPRO_KERF_HPP

#include "FloatSpinnerCommandInput.h"

namespace silvanus::generatebox::entities {
    struct Kerf {
        double value;
    };

    struct KerfParam {
        std::string expression;
    };

    struct KerfInput
    {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };
}

#endif //SILVANUSPRO_KERF_HPP
