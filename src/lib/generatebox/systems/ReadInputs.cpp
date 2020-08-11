//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/26/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "ReadInputs.hpp"
#include "entities/Dimensions.hpp"
#include "entities/Enabled.hpp"
#include "entities/EnableInput.hpp"
#include "entities/FingerWidth.hpp"
#include "entities/FingerWidthInput.hpp"
#include "entities/FingerPatternType.hpp"
#include "entities/FingerMode.hpp"
#include "entities/JointProfile.hpp"
#include "entities/Kerf.hpp"
#include "entities/KerfInput.hpp"
#include "entities/MaxOffset.hpp"
#include "entities/MaxOffsetInput.hpp"
#include "entities/ThicknessInput.hpp"
#include "entities/ToggleableThicknessInput.hpp"

#include <map>

using namespace silvanus::generatebox::systems;
using namespace silvanus::generatebox::entities;

void ReadInputs::execute() {
    readDimensionsInputs();
    readKerfInputs();
    readFingerWidthInputs();
    readMaxOffsetInputs();
    readFingerTypeInputs();
    readEnableInputs();
}

void ReadInputs::readDimensionsInputs() {
    auto dimension_view = m_registry.view<Dimensions, DimensionsInputs>();
    dimension_view.each(
        [](auto &dimensions, auto const &inputs) {
            dimensions.length = inputs.length_control->value();
            dimensions.width  = inputs.width_control->value();
            dimensions.height = inputs.height_control->value();
        }
    );

    auto thickness_view = m_registry.view<Dimensions, ToggleableThicknessInput>();
    thickness_view.each(
        [this](auto& dimensions, auto const& thickness) {
            dimensions.thickness = thickness.selector->value() ? thickness.enabled->value() : thickness.disabled->value();
        }
    );

    auto divider_view = m_registry.view<Dimensions, ThicknessInput>();
    divider_view.each(
        [this](auto& dimensions, auto const& thickness) {
            dimensions.thickness = thickness.control->value();
        }
    );
}

void ReadInputs::readKerfInputs() {
    auto view = m_registry.view<KerfInput>();

    view.each(
        [this](auto entity, auto const &kerf) {
            if (kerf.control->value() == 0) {
                m_registry.remove_if_exists<Kerf>(entity);
                return;
            }
            m_registry.emplace_or_replace<Kerf>(
                entity, kerf.control->value()
            );
        }
    );
}

void ReadInputs::readFingerWidthInputs() {
    auto view = m_registry.view<FingerWidth, FingerWidthInput>();

    view.each(
        [](auto &width, auto const &input) {
            width.value = input.control->value();
        }
    );
}

void ReadInputs::readFingerTypeInputs() {
    std::map<FingerMode, std::function<void(entt::entity)>> finger_mode_selector = {
        {
            FingerMode::Automatic,
            [this](auto entity) {
                this->m_registry.emplace_or_replace<AutomaticFingerPatternType>(entity);
            }
        },
        {
            FingerMode::Constant,
            [this](auto entity) {
                this->m_registry.emplace_or_replace<ConstantFingerPatternType>(entity);
            }
        },
        {
            FingerMode::ConstantAdaptive,
            [this](auto entity) {
                this->m_registry.emplace_or_replace<ConstantAdaptiveFingerPatternType>(entity);
            }
        },
        {
            FingerMode::None,
            [this](auto entity) {
                this->m_registry.emplace_or_replace<NoFingerPatternType>(entity);
            }
        }
    };

    auto view = m_registry.view<FingerPatternType, JointProfile, FingerPatternInput>();
    view.each(
        [&](auto entity, auto &pattern, auto & profile, auto const &input) {
            auto finger_mode = static_cast<FingerMode>(input.control->selectedItem()->index());
            pattern.value = finger_mode;
            profile.finger_type = finger_mode;
            finger_mode_selector[pattern.value](entity);
        }
    );
}

void ReadInputs::readMaxOffsetInputs() {
    auto view = m_registry.view<MaxOffset, MaxOffsetInput>();

    view.each(
        [](auto &offset, auto const &input) {
            offset.value = input.control->value();
        }
    );
}

void ReadInputs::readEnableInputs() {
    auto view = m_registry.view<EnableInput>();

    view.each(
        [this](auto entity, auto const &enable_input) {
            auto enabled = enable_input.control->value();
            if (!enabled) {
                m_registry.remove_if_exists<Enabled>(entity);
                return;
            }

            m_registry.emplace_or_replace<Enabled>(entity, true);
        }
    );
}
