//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_JOINTPANEL_HPP
#define SILVANUSPRO_JOINTPANEL_HPP

#include "JointGroup.hpp"
#include "JointExtrusion.hpp"

namespace silvanus::generatebox::entities {
    struct JointPanel
    {
        JointGroup group;
        JointExtrusion extrusion;
    };
}

#endif //SILVANUSPRO_JOINTPANEL_HPP
