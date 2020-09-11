//
// Created by Hobbyist Maker on 9/13/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PANELDIALOGCONTROLS_HPP
#define SILVANUSPRO_PANELDIALOGCONTROLS_HPP

#include "entities/DialogInputs.hpp"
#include "entities/EntitiesAll.hpp"
#include "entities/InputConfig.hpp"

#include <entt/entt.hpp>

namespace silvanus::generatebox::dialog {

    class PanelDialogControls {

            entt::registry&   m_registry;
            entt::entity      m_entity;

            adsk::core::Ptr<adsk::core::BoolValueCommandInput>    m_enable;
            adsk::core::Ptr<adsk::core::BoolValueCommandInput>    m_override;
            adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> m_thickness;
            adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> m_thickness_default;

        public:
            PanelDialogControls(entt::registry& registry, PanelDefaultConfiguration& row, bool is_metric);
            PanelDialogControls(entt::registry& registry, entt::entity entity) : m_registry{registry}, m_entity{entity} {};

            void configure(void (&f)(entt::registry&, entt::entity)) {
                f(m_registry, m_entity);
            };

            auto addLabel(const adsk::core::Ptr<adsk::core::TextBoxCommandInput>& control) -> PanelDialogControls&;
            auto addEnable(const adsk::core::Ptr<adsk::core::BoolValueCommandInput>& control) -> PanelDialogControls&;
            auto addOverride(const adsk::core::Ptr<adsk::core::BoolValueCommandInput>& control) -> PanelDialogControls&;
            auto addThickness(const adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput>& control) -> PanelDialogControls&;
            auto addActiveThickness(const adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput>& control,
                                    std::string parameter) -> PanelDialogControls&;

    };
}

#endif //SILVANUSPRO_PANELDIALOGCONTROLS_HPP
