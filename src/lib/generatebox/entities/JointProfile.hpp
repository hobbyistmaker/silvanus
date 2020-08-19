//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_JOINTPROFILE_HPP
#define SILVANUSPRO_JOINTPROFILE_HPP

#include "entities/AxisFlag.hpp"
#include "entities/FingerMode.hpp"
#include "entities/Position.hpp"
#include "entities/JointType.hpp"
#include "entities/PanelType.hpp"

namespace silvanus::generatebox::entities {
    struct JointProfile {
        Position   panel_position   = Position::Outside;
        Position   joint_position   = Position::Outside;
        JointType  joint_type       = JointType::Normal;
        FingerMode finger_type      = FingerMode::Automatic;
        int        finger_count     = 0;
        double     finger_width     = 0;
        double     pattern_distance = 0;
        double     pattern_offset   = 0;
        double     finger_offset    = 0;
        AxisFlag   panel_orientation = AxisFlag::Length;
        AxisFlag   joint_orientation = AxisFlag::Height;
    };

    struct OutsideJointProfile : public JointProfile {
    };
    struct InsideJointProfile : public JointProfile {
    };

    struct CompareJointProfile {
        bool operator()(const JointProfile &lhs, const JointProfile &rhs) const {
            auto lhs_is_less = (lhs.joint_type < rhs.joint_type) && (lhs.finger_count < rhs.finger_count) && (lhs.finger_width < rhs.finger_width) &&
                               (lhs.pattern_distance < rhs.pattern_distance) && (lhs.finger_type < rhs.finger_type) &&
                               (lhs.joint_orientation < rhs.joint_orientation) &&
                               (lhs.panel_orientation < rhs.panel_orientation);
            return lhs_is_less;
        }
    };

}

#endif //SILVANUSPRO_JOINTPROFILE_HPP
