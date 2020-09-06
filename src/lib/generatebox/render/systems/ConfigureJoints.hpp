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

#include "render/systems/joints/render_joint_systems.hpp"

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include <entt/entt.hpp>
#include <unordered_map>

namespace silvanus::generatebox::systems {

    class ConfigureJoints
    {
            adsk::core::Ptr<adsk::core::Application> m_app;
            entt::registry &m_registry;

        public:
            ConfigureJoints(
                const adsk::core::Ptr<adsk::core::Application>& app,
                entt::registry &registry
            ) : m_app{app}, m_registry{registry} {};

            void execute() {
                updateJointPatternDistances(m_registry);
                initializeNormalAutomaticWidthBoxJointPatternValues(m_registry);
                initializeInverseAutomaticWidthBoxJointPatternValues(m_registry);
                initializeAllConstantWidthBoxJointPatternValues(m_registry);
                updateInverseConstantWidthBoxJointPatternValues(m_registry);

                initializeNormalTenonJointPatternValues(m_registry);
                initializeInverseTenonJointPatternValues(m_registry);
                initializeNormalDoubleTenonJointPatternValues(m_registry);
                initializeInverseDoubleTenonJointPatternValues(m_registry);
                initializeNormalTripleTenonJointPatternValues(m_registry);
                initializeInverseTripleTenonJointPatternValues(m_registry);
                initializeNormalQuadTenonJointPatternValues(m_registry);
                initializeInverseQuadTenonJointPatternValues(m_registry);

                initializeNormalLapJointPatternValues(m_registry);
                initializeInverseLapJointPatternValues(m_registry);

                initializeInverseTrimJointPatternValues(m_registry);

                kerfAdjustJointPatternValues(m_registry);

                updateJointProfiles(m_registry);
                updateJointProfileGroups(m_registry);
                addJointGroups(m_registry);
            };

    };

}


#endif /* silvanuspro_configurejoints_hpp */
