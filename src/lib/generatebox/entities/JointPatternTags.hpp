//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_JOINTPATTERNTAGS_HPP
#define SILVANUSPRO_JOINTPATTERNTAGS_HPP

#include "Position.hpp"

namespace silvanus::generatebox::entities {
    struct JointPatternTag { Position position; };
    struct BottomLapJointPattern : public JointPatternTag{};
    struct CornerJointPattern : public JointPatternTag{};
    struct InverseJointPattern : public JointPatternTag{};
    struct MortiseJointPattern : public JointPatternTag{};
    struct NoJointPattern : public JointPatternTag{};
    struct NormalJointPattern : public JointPatternTag{};
    struct TenonJointPattern : public JointPatternTag{};
    struct TopLapJointPattern : public JointPatternTag{};
    struct TrimJointPattern : public JointPatternTag{};
}

#endif //SILVANUSPRO_JOINTPATTERNTAGS_HPP
