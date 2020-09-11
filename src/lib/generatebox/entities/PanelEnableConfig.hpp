//
// Created by Hobbyist Maker on 9/12/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PANELENABLECONFIG_HPP
#define SILVANUSPRO_PANELENABLECONFIG_HPP

#include <string>
#include <lib/generatebox/entities/DialogInputs.hpp>

namespace silvanus::generatebox::entities {

    struct PanelEnableConfig {
        entities::DialogInputs lookup;
        std::string            id;
        std::string            name;
        bool                   default_value;
    };

}
#endif //SILVANUSPRO_PANELENABLECONFIG_HPP
