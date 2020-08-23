//
// Created by Hobbyist Maker on 8/20/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_INPUTCONFIG_HPP
#define SILVANUSPRO_INPUTCONFIG_HPP

#include <string>

#include "entities/DialogInputs.hpp"

namespace silvanus::generatebox::dialog {
    struct InputConfig {
        entities::DialogInputs lookup;
        std::string            parameter;
        std::string            id;
        std::string            name;
        std::string            unit_type;
        double                 minimum;
        double                 maximum;
        double                 step;
        double                 initial_value;
    };
}
#endif //SILVANUSPRO_INPUTCONFIG_HPP
