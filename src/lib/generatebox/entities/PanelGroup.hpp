//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_PANELGROUP_HPP
#define SILVANUSPRO_PANELGROUP_HPP

#include "entities/AxisFlag.hpp"
#include "entities/Dimensions.hpp"
#include "entities/ExtrusionDistance.hpp"
#include "entities/PanelProfile.hpp"
#include "entities/Position.hpp"

namespace silvanus::generatebox::entities {
    struct PanelGroup {
        AxisFlag orientation = AxisFlag::Length;
        PanelProfile profile;
        ExtrusionDistance distance;
        Position position = Position::Outside;
    };
}

#endif //SILVANUSPRO_PANELGROUP_HPP
