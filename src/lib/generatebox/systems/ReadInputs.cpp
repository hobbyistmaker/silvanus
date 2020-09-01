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
#include "entities/FingerPattern.hpp"
#include "entities/JointProfile.hpp"
#include "entities/Kerf.hpp"
#include "entities/KerfInput.hpp"
#include "entities/MaxOffset.hpp"
#include "entities/MaxOffsetInput.hpp"
#include "entities/Thickness.hpp"
#include "entities/ToggleableThicknessInput.hpp"

#include "plog/Log.h"
#include "FingerPattern.hpp"

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
    auto dimensions_view = m_registry.view<Dimensions, const DimensionsInputs>().proxy();
    for (auto &&[entity, dimensions, inputs]: dimensions_view) {
        PLOG_DEBUG << "Reading panel dimensions" << inputs.length->value() << ", " << inputs.width->value() << ", " << inputs.height->value();
        dimensions.length = inputs.length->value();
        dimensions.width  = inputs.width->value();
        dimensions.height = inputs.height->value();
    }

    auto toggle_thickness_view = m_registry.view<Dimensions, const ToggleableThicknessInput>().proxy();
    for (auto &&[entity, dimensions, thickness]: toggle_thickness_view) {
        dimensions.thickness = thickness.selector->value() ? thickness.enabled->value() : thickness.disabled->value();
    }

    auto thickness_view = m_registry.view<Dimensions, const ThicknessInput>().proxy();
    for (auto &&[entity, dimensions, thickness]: thickness_view) {
        PLOG_DEBUG << "Reading panel thickness " << thickness.control->value();
        dimensions.thickness = thickness.control->value();
    }
}

void ReadInputs::readKerfInputs() {
    auto view = m_registry.view<const KerfInput>();
    auto proxy = view.proxy();
    PLOG_DEBUG << "Kerf view size: " << view.size();

    for (auto &&[entity, kerf]: proxy) {
        PLOG_DEBUG << "Reading kerf input " << kerf.control->value();

        if (kerf.control->value() == 0) {
            m_registry.remove_if_exists<Kerf>(entity);
            continue;
        }

        m_registry.emplace_or_replace<Kerf>( entity, kerf.control->value() );
    }
    PLOG_DEBUG << "Finished processing kerf inputs";
}

void ReadInputs::readFingerWidthInputs() {
    PLOG_DEBUG << "Started readFingerWidthInputs";
    auto view = m_registry.view<FingerWidth, const FingerWidthInput>().proxy();
    for (auto &&[entity, width, input]: view) {
        if (!input.control) { PLOG_DEBUG << "FingerWidthInput invalid"; }
        PLOG_DEBUG << "reading FingerWidth " << input.control->value();
        width.value = input.control->value();
    }
    PLOG_DEBUG << "Finished readFingerWidthInputs";
}

void ReadInputs::readFingerTypeInputs() {
    PLOG_DEBUG << "Started readFingerTypeInputs";
    std::map<FingerPatternType, std::function<void(entt::entity)>> finger_mode_selector = {
        {
            FingerPatternType::Automatic,
            [this](auto entity) {
                this->m_registry.emplace_or_replace<AutomaticFingerPatternType>(entity);
            }
        },
        {
            FingerPatternType::Constant,
            [this](auto entity) {
                this->m_registry.emplace_or_replace<ConstantFingerPatternType>(entity);
            }
        },
        {
            FingerPatternType::ConstantAdaptive,
            [this](auto entity) {
                this->m_registry.emplace_or_replace<ConstantAdaptiveFingerPatternType>(entity);
            }
        },
        {
            FingerPatternType::None,
            [this](auto entity) {
                this->m_registry.emplace_or_replace<NoFingerPatternType>(entity);
            }
        }
    };

    auto view = m_registry.view<FingerPattern, JointProfile, const FingerPatternInput>().proxy();
    for (auto &&[entity, pattern, profile, input]: view) {
        PLOG_DEBUG << " setting finger mode to " << input.control->selectedItem()->index();

        auto finger_mode = static_cast<FingerPatternType>(input.control->selectedItem()->index());
        pattern.value = finger_mode;
        profile.finger_type = finger_mode;
        finger_mode_selector[pattern.value](entity);
    }
    PLOG_DEBUG << "Finished readFingerTypeInputs";
}

void ReadInputs::readMaxOffsetInputs() {
    PLOG_DEBUG << "Started readMaxOffsetInputs";
    auto view = m_registry.view<MaxOffset, const MaxOffsetInput>().proxy();
    for (auto &&[entity, offset, input]: view) {
        PLOG_DEBUG << " setting max offset to " << input.control->value();

        offset.value = input.control->value();
    }
    PLOG_DEBUG << "Finished readMaxOffsetInputs";
}

void ReadInputs::readEnableInputs() {
    PLOG_DEBUG << "Started readEnableInputs";
    auto view = m_registry.view<const EnableInput>().proxy();
    for (auto &&[entity, enable_input]: view) {
        auto enabled = enable_input.control->value();

        if (!enabled) {
            PLOG_DEBUG << " Removing enabled flag from entity.";
            m_registry.remove_if_exists<Enabled>(entity);
            continue;
        }

        PLOG_DEBUG << " Adding enabled flag to entity.";
        m_registry.emplace_or_replace<Enabled>(entity, true);
    }
    PLOG_DEBUG << "Finished readEnableInputs";
}
