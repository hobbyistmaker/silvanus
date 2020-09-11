//
// Created by Hobbyist Maker on 8/27/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_JOINTPOSITION_HPP
#define SILVANUSPRO_JOINTPOSITION_HPP

#include "entities/Position.hpp"

namespace silvanus::generatebox::entities {

    struct JointPosition {
        Position value;
    };

    struct JointPositions {
        Position first;
        Position second;
    };
}
#endif //SILVANUSPRO_JOINTPOSITION_HPP
