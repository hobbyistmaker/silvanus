//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/27/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_CONFIGUREPANELS_HPP
#define SILVANUSPRO_CONFIGUREPANELS_HPP

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include <entt/entt.hpp>

namespace silvanus::generatebox::systems
{
    class ConfigurePanels
    {
            adsk::core::Ptr<adsk::core::Application> m_app;
            entt::registry &m_registry;

            std::unordered_map<std::string, adsk::fusion::DistanceUnits> m_units = {
                    {"cm", adsk::fusion::CentimeterDistanceUnits},
                    {"ft", adsk::fusion::FootDistanceUnits},
                    {"in", adsk::fusion::InchDistanceUnits},
                    {"m", adsk::fusion::MeterDistanceUnits},
                    {"mm", adsk::fusion::MillimeterDistanceUnits}
            };

            void updateEndReferencePoints();
            void updateExtrusionDistances();
            void addJointExtrusions();
            void updatePanelOffsets();
            void updatePanelProfiles();
            void updateStartReferencePoints();
            void addPanelExtrusions();
            void addPanelGroups();

        public:
            ConfigurePanels(
                adsk::core::Ptr<adsk::core::Application> app,
                entt::registry &registry
            ) : m_app{app}, m_registry{registry} {};

            void execute();

    };
}

#endif /* silvanuspro_configurepanels_hpp */
