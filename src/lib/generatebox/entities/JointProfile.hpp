//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_JOINTPROFILE_HPP
#define SILVANUSPRO_JOINTPROFILE_HPP

#include "entities/AxisFlag.hpp"
#include "entities/FingerMode.hpp"
#include "entities/JointDirection.hpp"
#include "entities/JointPattern.hpp"
#include "entities/PanelType.hpp"
#include "entities/Position.hpp"

#include "boost/functional/hash.hpp"

namespace silvanus::generatebox::entities {
    struct JointProfile {
        Position           panel_position    = Position::Outside;
        Position           joint_position    = Position::Outside;
        JointDirectionType joint_direction   = JointDirectionType::Normal;
        JointPatternType   joint_type        = JointPatternType::BoxJoint;
        FingerMode         finger_type       = FingerMode::Automatic;
        int                finger_count      = 0;
        double             finger_width      = 0;
        double             pattern_distance  = 0;
        double             pattern_offset    = 0;
        double             finger_offset     = 0;
        AxisFlag           panel_orientation = AxisFlag::Length;
        AxisFlag           joint_orientation = AxisFlag::Height;
        double             corner_width      = 0;
        double             corner_distance   = 0;

        bool operator<(const JointProfile &rhs) const {
            auto lhs_is_less =
                     (panel_position < rhs.panel_position) &&
                     (joint_position < rhs.joint_position) &&
                     (joint_direction < rhs.joint_direction) &&
                     (joint_type < rhs.joint_type) &&
                     (finger_type < rhs.finger_type) &&
                     (joint_orientation < rhs.joint_orientation) &&
                     (panel_orientation < rhs.panel_orientation) &&
                     (pattern_distance < rhs.pattern_distance) &&
                     (pattern_offset < rhs.pattern_offset) &&
                     (finger_count < rhs.finger_count) &&
                     (finger_width < rhs.finger_width) &&
                     (finger_offset < rhs.finger_offset) &&
                     (corner_width < rhs.corner_width) &&
                     (corner_distance < rhs.corner_distance);
            return lhs_is_less;
        }
    };

    struct OutsideJointProfile : public JointProfile {
    };
    struct InsideJointProfile : public JointProfile {
    };

    struct CompareJointProfile {
        bool operator()(const JointProfile &lhs, const JointProfile &rhs) const {
            auto lhs_is_less =
                     (lhs.panel_position < rhs.panel_position) &&
                     (lhs.joint_position < rhs.joint_position) &&
                     (lhs.joint_direction < rhs.joint_direction) &&
                     (lhs.joint_type < rhs.joint_type) &&
                     (lhs.finger_type < rhs.finger_type) &&
                     (lhs.joint_orientation < rhs.joint_orientation) &&
                     (lhs.panel_orientation < rhs.panel_orientation) &&
                     (lhs.pattern_distance < rhs.pattern_distance) &&
                     (lhs.pattern_offset < rhs.pattern_offset) &&
                     (lhs.finger_count < rhs.finger_count) &&
                     (lhs.finger_width < rhs.finger_width) &&
                     (lhs.finger_offset < rhs.finger_offset) &&
                     (lhs.corner_width < rhs.corner_width) &&
                     (lhs.corner_distance < rhs.corner_distance);
            return lhs_is_less;
        }
    };

}

namespace std {
    template<>
    class hash<silvanus::generatebox::entities::JointProfile> {
        public:
            std::size_t operator()(silvanus::generatebox::entities::JointProfile const& k) const noexcept {
                using std::size_t;
                using std::hash;
                using std::string;
                using silvanus::generatebox::entities::AxisFlag;
                using silvanus::generatebox::entities::FingerMode;
                using silvanus::generatebox::entities::Position;
                using silvanus::generatebox::entities::JointDirectionType;
                using silvanus::generatebox::entities::JointPatternType;

                using boost::hash_value;
                using boost::hash_combine;

                auto seed = std::size_t{0};

                hash_combine(seed, hash_value(k.panel_position));
                hash_combine(seed, hash_value(k.joint_position));
                hash_combine(seed, hash_value(k.joint_direction));
                hash_combine(seed, hash_value(k.joint_type));
                hash_combine(seed, hash_value(k.finger_type));
                hash_combine(seed, hash_value(k.finger_count));
                hash_combine(seed, hash_value(k.finger_width));
                hash_combine(seed, hash_value(k.pattern_distance));
                hash_combine(seed, hash_value(k.pattern_offset));
                hash_combine(seed, hash_value(k.finger_offset));
                hash_combine(seed, hash_value(k.panel_orientation));
                hash_combine(seed, hash_value(k.joint_orientation));
                hash_combine(seed, hash_value(k.corner_width));
                hash_combine(seed, hash_value(k.corner_distance));
                return seed;
            }
    };
}

#endif //SILVANUSPRO_JOINTPROFILE_HPP
