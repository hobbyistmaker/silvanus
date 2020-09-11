//
// Created by Hobbyist Maker on 9/13/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "PanelDialogControls.hpp"
#include "entity_helpers.hpp"

#include "entities/Thickness.hpp"

#include <boost/algorithm/string.hpp>

#include <plog/Log.h>

using namespace silvanus::generatebox;
using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::dialog;

PanelDialogControls::PanelDialogControls(entt::registry &registry, PanelDefaultConfiguration& row, bool is_metric) : m_registry{registry} {
    PLOG_DEBUG << "Making " << row.label.name << " with priority " << row.priority << " and orientation " << (int)row.orientation;
    auto entity = makeDialogPanelEntity(registry, Position::Outside, row.label.name, row.priority, row.orientation);
    auto defaults = row.thickness.defaults.at(is_metric);

    registry.emplace<PanelEnableConfig>(entity, row.enable.lookup, row.enable.id, row.enable.name, row.enable.default_value);
    registry.emplace<PanelLabelConfig>(entity, row.label.id, row.label.name, row.label.title);
    registry.emplace<PanelOverrideConfig>(entity, row.override.lookup, row.override.id, row.override.name);
    registry.emplace<PanelThicknessConfig>(entity, row.thickness.lookup, row.thickness.id, row.thickness.name,
                                           defaults.unit_type, defaults.minimum, defaults.maximum, defaults.step, defaults.initial_value);

    m_entity = entity;
}

auto PanelDialogControls::addLabel(const adsk::core::Ptr<adsk::core::TextBoxCommandInput>& control) -> PanelDialogControls& {
    m_registry.emplace<PanelLabelInput>(m_entity, control);
    return *this;
}

auto PanelDialogControls::addEnable(const adsk::core::Ptr<adsk::core::BoolValueCommandInput>& control) -> PanelDialogControls& {
    m_registry.emplace<PanelEnableInput>(m_entity, control);
    m_enable = control;
    return *this;
}

auto PanelDialogControls::addOverride(const adsk::core::Ptr<adsk::core::BoolValueCommandInput>& control) -> PanelDialogControls& {
    m_registry.emplace<PanelOverrideInput>(m_entity, control);
    m_override = control;
    return *this;
}

auto PanelDialogControls::addThickness(const adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput>& control) -> PanelDialogControls& {
    m_registry.emplace<PanelThicknessInput>(m_entity, control);
    m_thickness = control;
    return *this;
}

auto PanelDialogControls::addActiveThickness(const adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput>& control, std::string parameter) -> PanelDialogControls& {
    m_registry.emplace<PanelThicknessActive>(m_entity, control);
    m_registry.emplace<ThicknessParameter>(m_entity, boost::algorithm::to_lower_copy(parameter), 0.0, "cm");
    m_thickness_default = control;
    return *this;
}
