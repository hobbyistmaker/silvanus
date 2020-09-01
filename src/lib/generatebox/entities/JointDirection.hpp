//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_JOINTDIRECTION_HPP
#define SILVANUSPRO_JOINTDIRECTION_HPP

#include "JointPattern.hpp"

namespace silvanus::generatebox::entities {

    enum class JointDirectionType {
            Normal, Inverted
    };

    struct JointDirection {
        JointDirectionType value;
    };

    struct JointDirections {
        JointDirectionType first;
        JointDirectionType second;
    };

    struct JointDirectionInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
        bool reverse = false;
    };

    struct InverseJointDirection : JointDirection {};
    struct NormalJointDirection : JointDirection {};

    struct OutsideJointPattern {
        JointPatternType value;
    };

    struct InsideJointPattern {
        JointPatternType value;
    };

}

namespace std {
    template<>
    class hash<silvanus::generatebox::entities::JointDirectionType> {
        public:
            std::size_t operator()(silvanus::generatebox::entities::JointDirectionType const& key) const noexcept {
                return (size_t) key;
            }
    };
}

#endif //SILVANUSPRO_JOINTDIRECTION_HPP
