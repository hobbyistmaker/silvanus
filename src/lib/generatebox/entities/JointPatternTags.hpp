//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_JOINTPATTERNTAGS_HPP
#define SILVANUSPRO_JOINTPATTERNTAGS_HPP

#include "Position.hpp"

namespace silvanus::generatebox::entities {
    struct JointPatternTag { Position position; };
    struct InverseJointPattern : public JointPatternTag{};
    struct NormalJointPattern : public JointPatternTag{};
    struct CornerJointPattern : public JointPatternTag{};
}

#endif //SILVANUSPRO_JOINTPATTERNTAGS_HPP
