//
// Created by Hobbyist Maker on 8/11/20.
//

#ifndef SILVANUSPRO_PANELPOSITION_HPP
#define SILVANUSPRO_PANELPOSITION_HPP

#include "Position.hpp"

namespace silvanus::generatebox::entities {

    struct PanelPosition {
            Position value;
    };

    struct PanelPositions {
        Position first;
        Position second;
    };

}
#endif //SILVANUSPRO_PANELPOSITION_HPP
