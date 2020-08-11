//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_JOINTPATTERNTYPE_HPP
#define SILVANUSPRO_JOINTPATTERNTYPE_HPP

#include "JointOrientation.hpp"
#include "JointType.hpp"
#include "PanelOrientation.hpp"
#include "Position.hpp"

namespace silvanus::generatebox::entities {

    struct JointPatternPosition {
        Position  panel_position;
        AxisFlag  panel_orientation;
        JointType joint_type;
        AxisFlag  joint_orientation;
        Position  joint_position;
    };

    struct OutsideJointPattern {
        JointType value;
    };

    struct InsideJointPattern {
        JointType value;
    };
}

#endif //SILVANUSPRO_JOINTPATTERNTYPE_HPP
