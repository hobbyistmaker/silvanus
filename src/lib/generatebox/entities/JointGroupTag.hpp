//
// Created by Hobbyist Maker on 8/31/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_JOINTGROUPTAG_HPP
#define SILVANUSPRO_JOINTGROUPTAG_HPP

#include "entities/JointProfile.hpp"

#include <set>

namespace silvanus::generatebox::entities {

    struct JointGroupTag {
        std::set<size_t> value;
    };

}

#endif //SILVANUSPRO_JOINTGROUPTAG_HPP
