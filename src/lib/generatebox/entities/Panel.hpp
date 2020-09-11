//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_PANEL_HPP
#define SILVANUSPRO_PANEL_HPP

#include <string>
#include "entities/AxisFlag.hpp"
#include "entities/PanelAxis.hpp"

namespace silvanus::generatebox::entities {
    struct Panel {
        std::string name;
        int         priority    = 0;
        AxisFlag    orientation = AxisFlag::Length;
        PanelAxis   axis;
    };
}

#endif //SILVANUSPRO_PANEL_HPP
