//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_JOINTPATTERN_HPP
#define SILVANUSPRO_JOINTPATTERN_HPP

#include <Core/CoreAll.h>

namespace silvanus::generatebox::entities {

    enum class JointPatternType {
        BoxJoint, LapJoint, Tenon, DoubleTenon, TripleTenon, QuadTenon, Trim, None
    };

    struct JointPattern {
        JointPatternType value;
    };

    struct JointPatternInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };
}

#endif //SILVANUSPRO_JOINTPATTERN_HPP
