//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/29/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PANELFEATURE_HPP
#define SILVANUSPRO_PANELFEATURE_HPP

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include "Dimensions.hpp"

namespace silvanus::generatebox::fusion {

    class PanelFeature
    {

            adsk::core::Ptr<adsk::fusion::ExtrudeFeature> m_panel;

        public:
            PanelFeature(adsk::core::Ptr<adsk::fusion::ExtrudeFeature> panel) : m_panel{panel} {};
            auto extrudeCopy(
                    const entities::ExtrusionDistance& distance,
                    const entities::PanelOffset& offset
            ) const -> adsk::core::Ptr<adsk::fusion::ExtrudeFeature>;
            auto extrudeCopy(
                const entities::ExtrusionDistance& distance,
                const entities::PanelOffset& offset,
                const entities::PanelOffset& start
            ) const -> adsk::core::Ptr<adsk::fusion::ExtrudeFeature>;
    };

}


#endif /* silvanuspro_panelfeature_hpp */
