//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_JOINTGROUP_HPP
#define SILVANUSPRO_JOINTGROUP_HPP

#include "AxisFlag.hpp"
#include "FingerPattern.hpp"
#include "JointGroupTag.hpp"
#include "JointPattern.hpp"
#include "JointProfile.hpp"
#include "JointThickness.hpp"
#include "Position.hpp"

namespace silvanus::generatebox::entities {
    struct JointGroup {
        JointGroupTag tag;
        JointProfile profile;
        JointThickness joint_thickness;
        Position panel;
        Position joint;
    };
}

#endif //SILVANUSPRO_JOINTGROUP_HPP
