//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/28/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_CONFIGUREJOINTS_HPP
#define SILVANUSPRO_CONFIGUREJOINTS_HPP

#include "entities/AxisFlag.hpp"
#include "entities/Dimensions.hpp"
#include "entities/EndReferencePoint.hpp"

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include <entt/entt.hpp>
#include <unordered_map>

namespace silvanus::generatebox::systems {

    class ConfigureJoints
    {
            std::unordered_map<
                entities::AxisFlag,
                std::unordered_map<
                    entities::AxisFlag,
                    std::function<entities::Dimension(entities::EndReferencePoint)>
                >
            > reference_selector = {
                {
                    entities::AxisFlag::Length, {{
                        entities::AxisFlag::Height,
                        [this](entities::EndReferencePoint point) -> entities::Dimension {
                            return point.width;
                        }},
                    {
                        entities::AxisFlag::Width,
                        [this](entities::EndReferencePoint point) -> entities::Dimension {
                            return point.height;
                        }},
                }},
                {
                    entities::AxisFlag::Width,  {{
                        entities::AxisFlag::Length,
                        [this](entities::EndReferencePoint point) -> entities::Dimension {
                            return point.height;
                        }},
                    {
                        entities::AxisFlag::Height,
                        [this](entities::EndReferencePoint point) -> entities::Dimension {
                            return point.length;
                        }}
                }},
                {
                    entities::AxisFlag::Height, {{
                        entities::AxisFlag::Length,
                        [this](entities::EndReferencePoint point) -> entities::Dimension {
                            return point.width;
                        }},
                    {
                        entities::AxisFlag::Width,
                        [this](entities::EndReferencePoint point) -> entities::Dimension {
                            return point.length;
                        }},
                }}};

            adsk::core::Ptr<adsk::core::Application> m_app;
            entt::registry &m_registry;

            void addJointGroups();
            void addJoints();
            void addFingerParameters();
            void updateJointProfiles();
            void updateJointPatternDistances();

        public:
            ConfigureJoints(
                const adsk::core::Ptr<adsk::core::Application>& app,
                entt::registry &registry
            ) : m_app{app}, m_registry{registry} {};

            void execute();

    };

}


#endif /* silvanuspro_configurejoints_hpp */
