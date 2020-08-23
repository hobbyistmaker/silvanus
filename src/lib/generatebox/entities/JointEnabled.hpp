//
// Created by Hobbyist Maker on 8/28/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_JOINTENABLED_HPP
#define SILVANUSPRO_JOINTENABLED_HPP

#include <Core/CoreAll.h>

namespace silvanus::generatebox::entities {

    struct JointEnabledInput {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> control;
    };

    struct JointEnabled {
        bool value = true;
    };

}

#endif //SILVANUSPRO_JOINTENABLED_HPP
