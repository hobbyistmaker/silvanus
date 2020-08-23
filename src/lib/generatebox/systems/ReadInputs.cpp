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

#include "plog/Log.h"

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
    m_registry.view<Dimensions, DimensionsInputs>().each(
        [](auto &dimensions, auto const &inputs) {
            PLOG_DEBUG << "Reading panel dimensions" << inputs.length->value() << ", " << inputs.width->value() << ", " << inputs.height->value();
            dimensions.length = inputs.length->value();
            dimensions.width  = inputs.width->value();
            dimensions.height = inputs.height->value();
        }
    );

    m_registry.view<Dimensions, ToggleableThicknessInput>().each(
        [](auto& dimensions, auto const& thickness) {
            dimensions.thickness = thickness.selector->value() ? thickness.enabled->value() : thickness.disabled->value();
        }
    );

    m_registry.view<Dimensions, ThicknessInput>().each(
        [](auto& dimensions, auto const& thickness) {
            PLOG_DEBUG << "Reading panel thickness " << thickness.control->value();
            dimensions.thickness = thickness.control->value();
        }
    );
}

void ReadInputs::readKerfInputs() {
    m_registry.view<KerfInput>().each(
        [this](auto entity, auto const &kerf) {
            PLOG_DEBUG << "Reading kerf input " << kerf.control->value();
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
    m_registry.view<FingerWidth, FingerWidthInput>().each(
        [](auto &width, auto const &input) {
            PLOG_DEBUG << "reading FingerWidth " << input.control->value();
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

    m_registry.view<FingerPatternType, JointProfile, FingerPatternInput>().each(
        [&](auto entity, auto &pattern, auto & profile, auto const &input) {
            PLOG_DEBUG << " setting finger mode to " << input.control->selectedItem()->index();

            auto finger_mode = static_cast<FingerMode>(input.control->selectedItem()->index());
            pattern.value = finger_mode;
            profile.finger_type = finger_mode;
            finger_mode_selector[pattern.value](entity);
        }
    );
}

void ReadInputs::readMaxOffsetInputs() {
    m_registry.view<MaxOffset, MaxOffsetInput>().each(
        [](auto &offset, auto const &input) {
            PLOG_DEBUG << " setting max offset to " << input.control->value();

            offset.value = input.control->value();
        }
    );
}

void ReadInputs::readEnableInputs() {
    m_registry.view<EnableInput>().each(
        [this](auto entity, auto const &enable_input) {
            auto enabled = enable_input.control->value();
            if (!enabled) {
                PLOG_DEBUG << " Removing enabled flag from entity.";
                m_registry.remove_if_exists<Enabled>(entity);
                return;
            }

            PLOG_DEBUG << " Adding enabled flag to entity.";
            m_registry.emplace_or_replace<Enabled>(entity, true);
        }
    );
}
