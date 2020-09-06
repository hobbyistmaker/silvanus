//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_PANELPROFILE_HPP
#define SILVANUSPRO_PANELPROFILE_HPP

#include "Dimensions.hpp"

namespace silvanus::generatebox::entities {
    struct PanelProfile {
        Dimension length;
        Dimension width;
    };

    struct ComparePanelProfile {
        bool operator()(const entities::PanelProfile& a, const entities::PanelProfile& b) const {
            return (a.length.value < b.length.value) & (a.width.value < b.width.value);
        };
    };

}

#endif //SILVANUSPRO_PANELPROFILE_HPP
