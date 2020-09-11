//
// Created by Hobbyist Maker on 9/12/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PANELTHICKNESSCONFIG_HPP
#define SILVANUSPRO_PANELTHICKNESSCONFIG_HPP

#include <string>
#include <unordered_map>

#include "entities/DialogInputs.hpp"

namespace silvanus::generatebox::entities {

    struct InputDefaults {
        const std::string unit_type;
        const float       minimum;
        const float       maximum;
        const float       step;
        const float       initial_value;
    };

    struct PanelThicknessConfig {
        entities::DialogInputs lookup;
        std::string            id;
        std::string            name;
        std::string            unit_type;
        float                  minimum;
        float                  maximum;
        float                  step;
        float                  initial_value;
    };
}
#endif //SILVANUSPRO_PANELTHICKNESSCONFIG_HPP
