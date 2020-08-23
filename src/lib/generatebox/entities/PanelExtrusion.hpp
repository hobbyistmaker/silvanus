//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_PANELEXTRUSION_HPP
#define SILVANUSPRO_PANELEXTRUSION_HPP

#include "Dimensions.hpp"
#include <string>

namespace silvanus::generatebox::entities {
    struct PanelExtrusion {
        ExtrusionDistance  distance;
        PanelOffset        offset;
        std::string        name;
    };

    struct OutsidePanelExtrusion : public PanelExtrusion {};
    struct InsidePanelExtrusion : public PanelExtrusion {};
}

#endif //SILVANUSPRO_PANELEXTRUSION_HPP
