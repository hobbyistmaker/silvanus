//
// Created by Hobbyist Maker on 9/11/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_LENGTH_HPP
#define SILVANUSPRO_LENGTH_HPP

#include <Core/UserInterface/FloatSpinnerCommandInput.h>

#include <string>

namespace silvanus::generatebox::entities {

    struct MinLength {
        double value;
    };

    struct MaxLength {
        double value;
    };

    struct MinLengthParam {
        std::string expression;
    };

    struct MaxLengthParam {
        std::string expression;
    };

    struct MinLengthInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct MaxLengthInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

}

#endif //SILVANUSPRO_LENGTH_HPP
