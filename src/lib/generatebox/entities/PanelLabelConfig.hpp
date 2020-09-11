//
// Created by Hobbyist Maker on 9/12/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PANELLABELCONFIG_HPP
#define SILVANUSPRO_PANELLABELCONFIG_HPP

#include <string>
#include <lib/generatebox/entities/DialogInputs.hpp>

namespace silvanus::generatebox::entities {

    struct PanelLabelConfig {
        std::string            id;
        std::string            name;
        std::string            title;
    };
}

#endif //SILVANUSPRO_PANELLABELCONFIG_HPP
