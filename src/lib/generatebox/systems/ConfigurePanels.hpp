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

#include "entities/AxisFlag.hpp"
#include "entities/AxisProfile.hpp"
#include "entities/Dimensions.hpp"
#include "entities/Point.hpp"

#include <map>

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

            void findJoints();
            void findWidthJoints(
                entt::entity entity, std::string first,
                entities::AxisProfile length_profile, entities::AxisProfile width_profile, entities::AxisProfile height_profile
            );
            void addJointExtrusions();
            void addPanelExtrusions();
            void addPanelGroups();
            void updateEndReferencePoints();
            void updateExtrusionDistances();
            void updatePanelOffsets();
            void updatePanelProfiles();
            void updateStartReferencePoints();

            std::map<entities::AxisFlag, std::function<std::tuple<entities::AxisProfile, entities::AxisProfile, entities::AxisProfile>(entities::Dimensions)>> dimensions_selector = {
                {
                    entities::AxisFlag::Length, [](entities::Dimensions d) {
                    return std::tuple<entities::AxisProfile, entities::AxisProfile, entities::AxisProfile>{
                        entities::AxisProfile{{0, 0, 0}, {d.width, d.height, 0}},
                        entities::AxisProfile{{d.length - d.thickness, 0, 0}, {d.length, d.height, 0}},
                        entities::AxisProfile{{d.length - d.thickness, 0, 0}, {d.length, d.width, 0}}
                    };
                }
                },
                {
                    entities::AxisFlag::Width, [](entities::Dimensions d) {
                    return std::tuple<entities::AxisProfile, entities::AxisProfile, entities::AxisProfile>{
                        entities::AxisProfile{{d.width - d.thickness, 0, 0}, {d.width, d.height, 0}},
                        entities::AxisProfile{{0, 0, 0}, {d.length, d.height, 0}},
                        entities::AxisProfile{{d.width - d.thickness, 0, 0}, {d.width, d.length, 0}}
                    };
                }
                },
                {
                    entities::AxisFlag::Height, [](entities::Dimensions d) {
                    return std::tuple<entities::AxisProfile, entities::AxisProfile, entities::AxisProfile>{
                        entities::AxisProfile{{0, d.height - d.thickness, 0}, {d.width, d.height, 0}},
                        entities::AxisProfile{{0, d.height - d.thickness, 0}, {d.length, d.height, 0}},
                        entities::AxisProfile{{0, 0, 0}, {d.length, d.width, 0}}
                    };
                }
                }
            };

        public:
            ConfigurePanels(
                adsk::core::Ptr<adsk::core::Application> app,
                entt::registry &registry
            ) : m_app{app}, m_registry{registry} {};

            void execute();

    };
}

#endif /* silvanuspro_configurepanels_hpp */
