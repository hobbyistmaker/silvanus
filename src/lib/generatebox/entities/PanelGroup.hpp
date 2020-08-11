//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_PANELGROUP_HPP
#define SILVANUSPRO_PANELGROUP_HPP

#include "AxisFlag.hpp"
#include "Dimensions.hpp"
#include "PanelProfile.hpp"
#include "Position.hpp"

namespace silvanus::generatebox::entities {
    struct PanelGroup {
        AxisFlag orientation;
        PanelProfile profile;
        ExtrusionDistance distance;
        Position position;
    };
}

#endif //SILVANUSPRO_PANELGROUP_HPP
