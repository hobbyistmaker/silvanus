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

#include "render/systems/panels/render_panels_systems.hpp"

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

            std::map<
                entities::AxisFlag,
                std::function<std::tuple<entities::AxisProfile, entities::AxisProfile, entities::AxisProfile>(entities::Dimensions)>
            > dimensions_selector = {
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
                updateEndReferencePointsFromDimensions(m_registry);
                updatePanelProfilesFromEndReferencePoints(m_registry);
                kerfAdjustPanelProfiles(m_registry);

                updateStartReferencePointsFromEndReferencePoints(m_registry);
                updatePanelOffsetsFromEndReferencePoints(m_registry);
                kerfAdjustOutsidePanelOffsets(m_registry);
                kerfAdjustInsidePanelOffsets(m_registry);
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
