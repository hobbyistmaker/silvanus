//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_JOINTTHICKNESS_HPP
#define SILVANUSPRO_JOINTTHICKNESS_HPP

namespace silvanus::generatebox::entities {
    struct JointThickness {
        double value = 0;
    };

    struct CompareJointThickness {
        bool operator()(const entities::JointThickness& lhs, const entities::JointThickness& rhs) const {
            return (lhs.value < rhs.value);
        }
    };
}

#endif //SILVANUSPRO_JOINTTHICKNESS_HPP
