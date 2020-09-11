//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_PANELMAXPOINT_HPP
#define SILVANUSPRO_PANELMAXPOINT_HPP

#include "Dimensions.hpp"

namespace silvanus::generatebox::entities {
    struct PanelMaxPoint {
        double length;
        double width;
        double height;
    };

    struct PanelMaxParam {
        std::string length;
        std::string width;
        std::string height;
    };
}

#endif //SILVANUSPRO_PANELMAXPOINT_HPP
