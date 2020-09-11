//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_DIALOGSYSTEMMANAGER_HPP
#define SILVANUSPRO_DIALOGSYSTEMMANAGER_HPP

#include <entt/entt.hpp>

#include "findJoints.hpp"
#include "initializePanelsFromUserOptions.hpp"
#include "projectPlanes.hpp"
#include "updateEnableValue.hpp"
#include "updateFingerPattern.hpp"
#include "updateFingerWidth.hpp"
#include "updateJointCollisionData.hpp"
#include "updateJointDirection.hpp"
#include "updateJointPattern.hpp"
#include "updateJointPlanes.hpp"
#include "updatePanelDimensions.hpp"
#include "updateThicknessDimension.hpp"

#include "lib/generatebox/entities/DialogInputs.hpp"

using silvanus::generatebox::entities::DialogPanelCollisionPair;
using silvanus::generatebox::entities::JointPanelPlanes;

namespace silvanus::generatebox::dialog {

    class DialogSystemManager {

            entt::registry& m_registry;

        public:
            explicit DialogSystemManager(entt::registry& registry) : m_registry{registry} {};

            template <class F1, class T>
            void findJoints(bool reverse=false) { findJointsImpl<F1, T>(m_registry, reverse); }

            void initializePanels(entt::registry& registry) {
                initializePanelsFromUserOptionsImpl(m_registry, registry);
            }

            void updateCollisions() {
                updateEnableValueImpl(m_registry);
                updatePanelThicknessImpl(m_registry);
                updatePanelDimensionsImpl(m_registry);
                projectPlanesImpl(m_registry);
                projectPlaneParamsImpl(m_registry);
            }

            template <class T, class P>
            void updateJointPatternInputs(AxisFlag orientation) { updateJointPatternInputsImpl<T, P>(m_registry, orientation); }

            template <class T>
            void updateJointDirection(Position panel, Position joint, JointDirectionType direction) {
                updateJointDirectionImpl<T>(m_registry, panel, joint, direction);
            }

            void postUpdate() {
                updateJointPlanesImpl(m_registry);
                updateJointCollisionDataImpl(m_registry);
                updateFingerPatternTypeImpl(m_registry);
                updateFingerWidthImpl(m_registry);
                updateJointPatternImpl(m_registry);
                updateJointDirectionImpl(m_registry);
            }
    };

}

#endif //SILVANUSPRO_DIALOGSYSTEMMANAGER_HPP
