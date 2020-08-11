//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_JOINTEXTRUSION_HPP
#define SILVANUSPRO_JOINTEXTRUSION_HPP

#include "Dimensions.hpp"
#include <string>

namespace silvanus::generatebox::entities {
    struct JointExtrusion {
        ExtrusionDistance  distance;
        PanelOffset offset;
        std::string        name;
    };

    struct InsideJointExtrusion : public JointExtrusion {};
    struct OutsideJointExtrusion : public JointExtrusion {};
}

#endif //SILVANUSPRO_JOINTEXTRUSION_HPP
