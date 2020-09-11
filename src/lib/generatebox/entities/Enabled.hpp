//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_ENABLED_HPP
#define SILVANUSPRO_ENABLED_HPP

#include <Core/UserInterface/BoolValueCommandInput.h>

namespace silvanus::generatebox::entities {
    struct Enabled { bool value = true; };

    struct EnableInput
    {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> control;
    };
}

#endif //SILVANUSPRO_ENABLED_HPP
