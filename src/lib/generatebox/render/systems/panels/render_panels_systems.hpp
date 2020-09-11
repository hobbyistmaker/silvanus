//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_RENDER_PANELS_SYSTEMS_HPP
#define SILVANUSPRO_RENDER_PANELS_SYSTEMS_HPP

#include <entt/entt.hpp>

void initializeJointEnabledFromInput(entt::registry& registry);
void initializeJointExtrusionFromThicknessOffsetAndName(entt::registry& registry);
void initializePanelExtrusionsFromOffsetAndDistance(entt::registry& registry);
void initializePanelGroupFromProfileOrientationAndPosition(entt::registry& registry);
void kerfAdjustInsideJointExtrusionOffset(entt::registry& registry);
void kerfAdjustInsideJointPanelOffsets(entt::registry& registry);
void kerfAdjustInsideJointThickness(entt::registry& registry);
void kerfAdjustInsidePanelOffsets(entt::registry& registry);
void kerfAdjustOutsideJointPanelOffsets(entt::registry& registry);
void kerfAdjustOutsidePanelOffsets(entt::registry& registry);
void kerfAdjustPanelProfiles(entt::registry& registry);
void logInitialJointProperties(entt::registry& registry);
void tagLengthOrientationPanels(entt::registry& registry);
void tagWidthOrientationPanels(entt::registry& registry);
void tagHeightOrientationPanels(entt::registry& registry);
void tagAutomaticFingerWidthJoints(entt::registry& registry);
void tagConstantFingerWidthJoints(entt::registry& registry);
void tagConstantFingerCountJoints(entt::registry& registry);
void tagNoFingerJoints(entt::registry& registry);
void tagNormalDirectionJoints(entt::registry& registry);
void tagInverseDirectionJoints(entt::registry& registry);
void tagBoxJointPatternJoints(entt::registry& registry);
void tagLapJointPatternJoints(entt::registry& registry);
void tagTenonPatternJoints(entt::registry& registry);
void tagDoubleTenonPatternJoints(entt::registry& registry);
void tagTripleTenonPatternJoints(entt::registry& registry);
void tagQuadTenonPatternJoints(entt::registry& registry);
void tagTrimPatternJoints(entt::registry& registry);
void updateExtrusionDistancesFromDimensions(entt::registry& registry);
void updateJointPanelOffsetsFromExpressions(entt::registry& registry);
void updateJointPatternPositionsFromPanelAndJointPositions(entt::registry& registry);
void updateJointProfilesFromJointDirections(entt::registry &registry);
void updateJointProfilesFromJointPatterns(entt::registry& registry);
void updateJointProfilesFromPanelAndJointOrientations(entt::registry &registry);
void updateJointProfilesFromPanelAndJointPositions(entt::registry &registry);
void updatePanelOffsetsFromPanelMinPoints(entt::registry& registry);
void updatePanelProfilesFromPanelMinPoints(entt::registry& registry);

#endif //SILVANUSPRO_RENDER_PANELS_SYSTEMS_HPP
