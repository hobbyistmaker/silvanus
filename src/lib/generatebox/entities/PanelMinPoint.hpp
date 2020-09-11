//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_PANELMINPOINT_HPP
#define SILVANUSPRO_PANELMINPOINT_HPP

#include "Dimensions.hpp"

namespace silvanus::generatebox::entities {
    struct PanelMinPoint {
        double length;
        double width;
        double height;
    };

    struct PanelMinParam {
        std::string length;
        std::string width;
        std::string height;
    };
}

#endif //SILVANUSPRO_PANELMINPOINT_HPP
