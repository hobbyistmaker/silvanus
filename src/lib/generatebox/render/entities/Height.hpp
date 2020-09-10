//
// Created by Hobbyist Maker on 9/11/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_HEIGHT_HPP
#define SILVANUSPRO_HEIGHT_HPP

#include <Core/UserInterface/FloatSpinnerCommandInput.h>

#include <string>

namespace silvanus::generatebox::entities {

    struct MinHeight {
        double value;
    };

    struct MaxHeight {
        double value;
    };

    struct MinHeightParam {
        std::string expression;
    };

    struct MaxHeightParam {
        std::string expression;
    };

    struct MinHeightInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct MaxHeightInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };
}

#endif //SILVANUSPRO_HEIGHT_HPP
