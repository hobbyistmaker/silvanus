//
// Created by Hobbyist Maker on 8/16/20.
//

#ifndef SILVANUSPRO_JOINTPATTERNPOSITION_HPP
#define SILVANUSPRO_JOINTPATTERNPOSITION_HPP

#include "entities/AxisFlag.hpp"
#include "entities/JointPattern.hpp"
#include "entities/Position.hpp"

namespace silvanus::generatebox::entities {
    struct JointPatternPosition {
        Position  panel_position;
        AxisFlag         panel_orientation;
        JointPatternType joint_type;
        AxisFlag         joint_orientation;
        Position  joint_position;
    };
}

#endif //SILVANUSPRO_JOINTPATTERNPOSITION_HPP
