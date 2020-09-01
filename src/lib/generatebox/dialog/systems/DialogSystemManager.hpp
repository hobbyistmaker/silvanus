//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_DIALOGSYSTEMMANAGER_HPP
#define SILVANUSPRO_DIALOGSYSTEMMANAGER_HPP

#include <entt/entt.hpp>

#include "findJoints.hpp"
#include "updateJointDirectionInputs.hpp"
#include "updateJointPatternInputs.hpp"

#include "DialogInputs.hpp"

using silvanus::generatebox::entities::DialogPanelCollisionPair;
using silvanus::generatebox::entities::DialogPanelJoint;

void initializePanelDimensionInputsImpl(entt::registry& registry);
void initializePanelOrientationsImpl(entt::registry& registry);

void projectHeightPlanesImpl(entt::registry& registry);
void projectLengthPlanesImpl(entt::registry& registry);
void projectWidthPlanesImpl(entt::registry& registry);

void updateEnableValueImpl(entt::registry& registry);
void updateFingerPatternTypeImpl(entt::registry& registry);
void updateFingerWidthImpl(entt::registry& registry);
void updateJointCollisionDataImpl(entt::registry& registry);
void updateJointDirectionImpl(entt::registry& registry);

template <class T>
void updateJointDirectionImpl(entt::registry& registry, Position panel_position, Position joint_position, JointDirectionType joint_direction) {
    auto view = registry.view<JointDirections, const PanelPositions, const JointPositions, const T>();
    PLOG_DEBUG << "Joint direction view size is " << view.size();
    for (auto &&[entity, joint_directions, panels, joints, filter]: view.proxy()) {
        auto directions = JointDirections{joint_directions};

        PLOG_DEBUG << "Updating divider joint direction";
        directions.first = (panels.first == panel_position && joints.first == joint_position) ? joint_direction : directions.first;
        directions.second = (panels.second == joint_position && joints.second == panel_position) ? static_cast<JointDirectionType>(!(bool)joint_direction) : directions.second;
    }
};

void updateJointPatternImpl(entt::registry& registry);
void updateJointPatternInputsImpl(entt::registry& registry);
void updateJointPlanesImpl(entt::registry& registry);
void updatePanelDimensionsImpl(entt::registry& registry);

namespace silvanus::generatebox::dialog {

    class DialogSystemManager {

            entt::registry& m_registry;

        public:
            explicit DialogSystemManager(entt::registry& registry) : m_registry{registry} {};

            void initialize() {
                initializePanelOrientationsImpl(m_registry);
                initializePanelDimensionInputsImpl(m_registry);
            }

            template <class F1, class F2, class T>
            void findJoints() { findJointsImpl<F1, F2, T>(m_registry); }

            void updateCollisions() {
                updatePanelDimensionsImpl(m_registry);
                projectLengthPlanesImpl(m_registry);
                projectWidthPlanesImpl(m_registry);
                projectHeightPlanesImpl(m_registry);
            }

            template <class T, class P>
            void updateJointPatternInputs() { updateJointPatternInputsImpl<T, P>(m_registry); }

            template <class T>
            void updateJointDirection(Position panel, Position joint, JointDirectionType direction) {
                updateJointDirectionImpl<T>(m_registry, panel, joint, direction);
            }

            template <class T, class P>
            void updateJointDirectionInputs() { updateJointDirectionInputsImpl<T, P>(m_registry); }

            void postUpdate() {
                updateJointPlanesImpl(m_registry);
                updateJointCollisionDataImpl(m_registry);
                updateEnableValueImpl(m_registry);
                updateFingerPatternTypeImpl(m_registry);
                updateFingerWidthImpl(m_registry);
                updateJointPatternImpl(m_registry);
                updateJointDirectionImpl(m_registry);
            }
    };

}

#endif //SILVANUSPRO_DIALOGSYSTEMMANAGER_HPP
