//
// Created by Hobbyist Maker on 9/11/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_WIDTH_HPP
#define SILVANUSPRO_WIDTH_HPP

#include <Core/UserInterface/FloatSpinnerCommandInput.h>

#include <string>

namespace silvanus::generatebox::entities {

    struct MinWidth {
        double value;
    };

    struct MaxWidth {
        double value;
    };

    struct MinWidthParam {
        std::string expression;
    };

    struct MaxWidthParam {
        std::string expression;
    };

    struct MinWidthInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct MaxWidthInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

}

#endif //SILVANUSPRO_WIDTH_HPP
