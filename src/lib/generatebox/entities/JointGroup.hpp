//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_JOINTGROUP_HPP
#define SILVANUSPRO_JOINTGROUP_HPP

#include "AxisFlag.hpp"
#include "FingerMode.hpp"
#include "JointProfile.hpp"
#include "JointThickness.hpp"
#include "JointPattern.hpp"
#include "Position.hpp"

namespace silvanus::generatebox::entities {
    struct JointGroup {
        JointProfile profile;
        JointThickness joint_thickness;
        Position panel;
        Position joint;
    };
}

#endif //SILVANUSPRO_JOINTGROUP_HPP
