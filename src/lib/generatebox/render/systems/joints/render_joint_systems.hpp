//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_RENDER_JOINT_SYSTEMS_HPP
#define SILVANUSPRO_RENDER_JOINT_SYSTEMS_HPP

#include <entt/entt.hpp>

void updateJointProfiles(entt::registry& registry);
void updateJointProfileGroups(entt::registry& registry);
void addJointGroups(entt::registry& registry);

void updateJointPatternDistances(entt::registry& registry);
void initializeNormalAutomaticWidthBoxJointPatternValues(entt::registry& registry);
void initializeInverseAutomaticWidthBoxJointPatternValues(entt::registry& registry);
void initializeAllConstantWidthBoxJointPatternValues(entt::registry& registry);
void updateInverseConstantWidthBoxJointPatternValues(entt::registry& registry);
void initializeNormalTenonJointPatternValues(entt::registry& registry);
void initializeInverseTenonJointPatternValues(entt::registry& registry);
void initializeNormalDoubleTenonJointPatternValues(entt::registry& registry);
void initializeInverseDoubleTenonJointPatternValues(entt::registry& registry);
void initializeNormalTripleTenonJointPatternValues(entt::registry& registry);
void initializeInverseTripleTenonJointPatternValues(entt::registry& registry);
void initializeNormalQuadTenonJointPatternValues(entt::registry& registry);
void initializeInverseQuadTenonJointPatternValues(entt::registry& registry);
void initializeNormalLapJointPatternValues(entt::registry& registry);
void initializeInverseLapJointPatternValues(entt::registry& registry);
void initializeInverseTrimJointPatternValues(entt::registry& registry);
void kerfAdjustJointPatternValues(entt::registry& registry);

#endif //SILVANUSPRO_RENDER_JOINT_SYSTEMS_HPP
