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
#include "entities/Dimensions.hpp"
#include "entities/Point.hpp"

#include "render/systems/panels/render_panels_systems.hpp"

#include <map>

namespace silvanus::generatebox::systems
{
    class ConfigurePanels
    {
            adsk::core::Ptr<adsk::core::Application> m_app;
            entt::registry &m_registry;

        public:
            ConfigurePanels(
                adsk::core::Ptr<adsk::core::Application> app,
                entt::registry &registry
            ) : m_app{app}, m_registry{registry} {};

            void execute() {
                initializeJointEnabledFromInput(m_registry);

                updateJointProfilesFromJointDirections(m_registry);
                updateJointProfilesFromPanelAndJointPositions(m_registry);
                updateJointProfilesFromPanelAndJointOrientations(m_registry);
                updateJointProfilesFromJointPatterns(m_registry);
                updateJointPatternPositionsFromPanelAndJointPositions(m_registry);

                tagLengthOrientationPanels(m_registry);
                tagWidthOrientationPanels(m_registry);
                tagHeightOrientationPanels(m_registry);
                tagAutomaticFingerWidthJoints(m_registry);
                tagConstantFingerWidthJoints(m_registry);
                tagConstantFingerCountJoints(m_registry);
                tagNoFingerJoints(m_registry);
                tagNormalDirectionJoints(m_registry);
                tagInverseDirectionJoints(m_registry);
                tagBoxJointPatternJoints(m_registry);
                tagLapJointPatternJoints(m_registry);
                tagTenonPatternJoints(m_registry);
                tagDoubleTenonPatternJoints(m_registry);
                tagTripleTenonPatternJoints(m_registry);
                tagQuadTenonPatternJoints(m_registry);
                tagTrimPatternJoints(m_registry);

                logInitialJointProperties(m_registry);

                updateExtrusionDistancesFromDimensions(m_registry);
                updatePanelProfilesFromPanelMinPoints(m_registry);
                kerfAdjustPanelProfiles(m_registry);

                updatePanelOffsetsFromPanelMinPoints(m_registry);
                kerfAdjustOutsidePanelOffsets(m_registry);
                kerfAdjustInsidePanelOffsets(m_registry);
                updateJointPanelOffsetsFromExpressions(m_registry);
                kerfAdjustOutsideJointPanelOffsets(m_registry);
                kerfAdjustInsideJointPanelOffsets(m_registry);

                initializePanelGroupFromProfileOrientationAndPosition(m_registry);
                initializePanelExtrusionsFromOffsetAndDistance(m_registry);
                kerfAdjustInsideJointThickness(m_registry);
                initializeJointExtrusionFromThicknessOffsetAndName(m_registry);
                kerfAdjustInsideJointExtrusionOffset(m_registry);
            }

    };
}

#endif /* silvanuspro_configurepanels_hpp */
