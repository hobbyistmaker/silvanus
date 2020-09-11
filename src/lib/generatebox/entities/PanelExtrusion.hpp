//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_PANELEXTRUSION_HPP
#define SILVANUSPRO_PANELEXTRUSION_HPP

#include "entities/Dimensions.hpp"
#include "entities/ExtrusionDistance.hpp"
#include "entities/PanelOffset.hpp"

#include <string>

namespace silvanus::generatebox::entities {
    struct PanelExtrusion {
        entt::entity       panel_id;
        ExtrusionDistance  distance;
        PanelOffset        offset;
        std::string        name;
    };

    struct PanelExtrusionParams {
        std::string distance;
        std::string offset;
    };

    struct OutsidePanelExtrusion : public PanelExtrusion {};
    struct InsidePanelExtrusion : public PanelExtrusion {};
}

#endif //SILVANUSPRO_PANELEXTRUSION_HPP
