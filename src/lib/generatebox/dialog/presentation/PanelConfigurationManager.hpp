//
// Created by Hobbyist Maker on 9/16/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PANELCONFIGURATIONMANAGER_HPP
#define SILVANUSPRO_PANELCONFIGURATIONMANAGER_HPP

#include <entt/entt.hpp>
#include "entities/DialogInputs.hpp"
#include "entities/EntitiesAll.hpp"
#include "entities/InputConfig.hpp"
#include "PanelDialogControls.hpp"

namespace silvanus::generatebox::dialog {

    using silvanus::generatebox::entities::Panel;
    using silvanus::generatebox::entities::PanelLabelConfig;
    using silvanus::generatebox::entities::PanelEnableConfig;
    using silvanus::generatebox::entities::PanelOverrideConfig;
    using silvanus::generatebox::entities::PanelThicknessConfig;
    using silvanus::generatebox::entities::PanelEnableInput;
    using silvanus::generatebox::entities::PanelOverrideInput;
    using silvanus::generatebox::entities::PanelThicknessInput;

    class PanelConfigurationManager {

            entt::registry& m_registry;
            bool            m_is_metric;

        public:

            PanelConfigurationManager(entt::registry& registry, bool is_metric) : m_registry{registry}, m_is_metric{is_metric} {};

            auto initializePanel(silvanus::generatebox::dialog::PanelDefaultConfiguration& config) { return PanelDialogControls(m_registry, config, m_is_metric); }

            auto panels() {
                return m_registry.view<Panel, PanelLabelConfig, PanelEnableConfig, PanelOverrideConfig, PanelThicknessConfig>().proxy();
            }
            auto allThicknessInputs() {
                return m_registry.view<PanelEnableInput, PanelOverrideInput, PanelThicknessInput>().proxy();
            }
            auto enableOverrideInputs() {
                return m_registry.view<PanelEnableInput, PanelOverrideInput>().proxy();
            }
            auto overrideThicknessInputs() {
                return m_registry.view<PanelOverrideInput, PanelThicknessInput>().proxy();
            }
            auto addControls(entt::entity entity) {
                return PanelDialogControls(m_registry, entity);
            }
    };

}

#endif //SILVANUSPRO_PANELCONFIGURATIONMANAGER_HPP
