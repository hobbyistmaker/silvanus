//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_ORIENTATIONGROUP_HPP
#define SILVANUSPRO_ORIENTATIONGROUP_HPP

#include "entities/AxisFlag.hpp"

namespace silvanus::generatebox::entities {
    struct OrientationGroup
    {
        AxisFlag panel;
        AxisFlag finger;
    };
}

#endif //SILVANUSPRO_ORIENTATIONGROUP_HPP
