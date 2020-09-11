//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_JOINTEXTRUSION_HPP
#define SILVANUSPRO_JOINTEXTRUSION_HPP

#include "entities/Dimensions.hpp"
#include "entities/JointThickness.hpp"
#include "entities/JointPanelOffset.hpp"
#include <string>

namespace silvanus::generatebox::entities {
    struct JointExtrusion {
        entt::entity       joint_id;
        JointThickness     distance;
        JointPanelOffset   offset;
        std::string        name;
    };

    struct JointExtrusionParams {
        std::string distance;
        std::string offset;
    };

    struct InsideJointExtrusion : public JointExtrusion {};
    struct OutsideJointExtrusion : public JointExtrusion {};
}

#endif //SILVANUSPRO_JOINTEXTRUSION_HPP
