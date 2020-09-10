//
// Created by Hobbyist Maker on 9/3/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_CREATEPANELOVERRIDEROW_HPP
#define SILVANUSPRO_CREATEPANELOVERRIDEROW_HPP

#include "entities/AxisFlag.hpp"
#include "lib/generatebox/dialog/entities/DialogInputs.hpp"
#include "entities/EntitiesAll.hpp"

#include <plog/Log.h>

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include <entt/entt.hpp>
#include <utility>

namespace silvanus::generatebox::dialog {

    using adsk::core::Ptr;
    using adsk::core::BoolValueCommandInput;
    using adsk::core::CommandInput;
    using adsk::core::FloatSpinnerCommandInput;
    using adsk::core::TextBoxCommandInput;

    using entities::AxisFlag;
    using entities::DialogPanelEnable;
    using entities::DialogPanelInputs;
    using entities::DialogPanelLabel;
    using entities::DialogPanelOverride;
    using entities::DialogPanelOverrideThickness;
    using entities::DialogPanelThickness;
    using entities::DialogPanel;
    using entities::DialogPanelPlanes;
    using entities::DialogThicknessInput;
    using entities::DialogPanelEnableValue;
    using entities::OutsidePanel;
    using entities::Panel;
    using entities::PanelPosition;
    using entities::PanelDimensions;
    using entities::Position;

    using boolCommandInputPtr  = Ptr<BoolValueCommandInput>;
    using commandInputPtr      = Ptr<CommandInput>;
    using floatCommandInputPtr = Ptr<FloatSpinnerCommandInput>;
    using textCommandInputPtr  = Ptr<TextBoxCommandInput>;

    class CreatePanelOverrideRow {

            entt::registry* m_registry;
            entt::entity    m_entity;

            const std::string& m_name;
            const int          m_priority;
            const AxisFlag     m_orientation;

        public:
            CreatePanelOverrideRow(entt::registry* registry, const std::string& name, const int priority, const AxisFlag orientation)
            : m_registry{registry},
              m_name{name},
              m_priority{priority},
              m_orientation{orientation}
            {
                m_entity = entt::null;
            };

            template <class T, class U, class V>
            auto createRow(
                const floatCommandInputPtr& parent, // NOLINT(performance-unnecessary-value-param)
                const textCommandInputPtr& label,
                const boolCommandInputPtr& enabled,
                const boolCommandInputPtr& override,
                const floatCommandInputPtr& thickness
            ) -> std::function<void(entt::registry&)> {

                PLOG_DEBUG << "Creating new row entity";

                m_entity = m_registry->create();
                m_registry->emplace<DialogPanelEnable>(m_entity, enabled);
                m_registry->emplace<DialogPanelInputs>(m_entity, label, enabled, override, parent);
                m_registry->emplace<DialogPanelLabel>(m_entity, label);
                m_registry->emplace<DialogPanelOverride>(m_entity, override);
                m_registry->emplace<DialogPanelOverrideThickness>(m_entity, thickness);
                m_registry->emplace<DialogPanelThickness>(m_entity, parent);
                m_registry->emplace<OutsidePanel>(m_entity);
                m_registry->emplace<PanelPosition>(m_entity, Position::Outside);
                m_registry->emplace<DialogPanel>(m_entity, m_name, m_priority, m_orientation);
                m_registry->emplace<DialogPanelPlanes>(m_entity);
                m_registry->emplace<PanelDimensions>(m_entity);
                m_registry->emplace<DialogPanelEnableValue>(m_entity);
                m_registry->emplace<Panel>(m_entity, m_name, m_priority, m_orientation);
                m_registry->emplace<V>(m_entity, parent);

                m_registry->set<T>(label, enabled, override, parent);
                m_registry->set<U>(parent);

                return [entity = this->m_entity, parent, label, enabled, override, thickness](entt::registry& registry) {
                    auto t_control = commandInputPtr{thickness};
                    auto d_control = commandInputPtr{parent};

                    auto toggle = override->value() ? t_control : d_control;

                    registry.replace<DialogPanelInputs>(entity, label, enabled, override, toggle);
                    registry.replace<DialogPanelThickness>(entity, toggle);
                    registry.replace<V>(entity, toggle);
                    registry.set<T>(label, enabled, override, toggle);
                    registry.set<U>(toggle);
                };
            }

            template <class T>
            void save() {
                m_registry->set<T>(m_entity);
            }
    };

}

#endif //SILVANUSPRO_CREATEPANELOVERRIDEROW_HPP
