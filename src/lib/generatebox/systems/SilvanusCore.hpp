//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/26/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_SILVANUSCORE_HPP
#define SILVANUSPRO_SILVANUSCORE_HPP

#include <Core/CoreAll.h>
#include <Fusion/Components/Component.h>

#include <entt/entt.hpp>

namespace silvanus::generatebox::systems {

    class SilvanusCore
    {

            adsk::core::Ptr<adsk::core::Application> m_app;
            entt::registry& m_registry;

        public:
            SilvanusCore(const adsk::core::Ptr<adsk::core::Application>& app, entt::registry& registry)
                : m_app{app}, m_registry{registry} {};

            void execute(
                    adsk::core::DefaultModelingOrientations orientation,
                    const adsk::core::Ptr<adsk::fusion::Component>& component,
                    bool is_parametric
            );
            void update(
                    adsk::core::DefaultModelingOrientations orientation,
                    adsk::core::Ptr<adsk::fusion::Component> component
            );
            void fast_preview(
                    adsk::core::DefaultModelingOrientations orientation,
                    const adsk::core::Ptr<adsk::fusion::Component>& component
            );
            void full_preview(
                adsk::core::DefaultModelingOrientations orientation,
                const adsk::core::Ptr<adsk::fusion::Component>& component
            );

    };

}


#endif /* silvanuspro_core_hpp */
