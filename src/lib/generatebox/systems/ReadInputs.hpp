//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/26/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_READINPUTS_HPP
#define SILVANUSPRO_READINPUTS_HPP

#include "entities/Dimensions.hpp"

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <entt/entt.hpp>

namespace silvanus::generatebox::systems {

    class ReadInputs {

            adsk::core::Ptr<adsk::core::Application> m_app;
            entt::registry &m_registry;

        public:
            ReadInputs(const adsk::core::Ptr<adsk::core::Application>& app, entt::registry& registry) :
                m_app{app}, m_registry{registry} {
            };

            void execute();
            void readEnableInputs();
            void readFingerWidthInputs();
            void readKerfInputs();
            void readMaxOffsetInputs();
            void readDimensionsInputs();
            void readFingerTypeInputs();
    };

}

#endif /* silvanuspro_readinputs_hpp */
