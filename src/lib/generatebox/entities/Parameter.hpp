//
// Created by Hobbyist Maker on 9/14/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PARAMETER_HPP
#define SILVANUSPRO_PARAMETER_HPP

#include <Core/UserInterface/FloatSpinnerCommandInput.h>

#include <string>

using floatSpinnerPtr = adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput>;

namespace silvanus::generatebox::entities {

    struct FloatParameterInput {
        std::string     name = "parameter";
        floatSpinnerPtr control;
    };

    struct FloatParameter {
        std::string name       = "parameter";
        double value           = 0.0;
        std::string expression = "";
        std::string unit_type  = "cm";
    };

    struct CountParameter {
        std::string name      = "parameter";
        int value             = 0;
    };

}

#endif //SILVANUSPRO_PARAMETER_HPP
