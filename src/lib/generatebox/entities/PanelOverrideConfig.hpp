//
// Created by Hobbyist Maker on 9/12/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PANELOVERRIDECONFIG_HPP
#define SILVANUSPRO_PANELOVERRIDECONFIG_HPP

#include <string>
#include <lib/generatebox/entities/DialogInputs.hpp>

namespace silvanus::generatebox::entities {

    struct PanelOverrideConfig {
        entities::DialogInputs lookup;
        std::string            id;
        std::string            name;
    };
}

#endif //SILVANUSPRO_PANELOVERRIDECONFIG_HPP
