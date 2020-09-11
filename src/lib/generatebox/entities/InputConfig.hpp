//
// Created by Hobbyist Maker on 8/20/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_INPUTCONFIG_HPP
#define SILVANUSPRO_INPUTCONFIG_HPP

#include <string>

#include "entities/EntitiesAll.hpp"

namespace silvanus::generatebox::dialog {

    struct InputConfig {
        entities::DialogInputs lookup;
        std::string            parameter;
        std::string            id;
        std::string            name;
        std::string            unit_type;
        double                 minimum;
        double                 maximum;
        double                 step;
        double                 initial_value;
    };

    struct ThicknessInputConfig {
        const entities::DialogInputs                            lookup;
        const std::string                                       parameter;
        const std::string                                       id;
        const std::string                                       name;
        const std::unordered_map<bool, entities::InputDefaults> defaults;
    };

    struct EnabledInputConfig {
        const entities::DialogInputs lookup;
        const std::string            id;
        const std::string            name;
        const bool                   default_value;
    };

    struct OverrideInputConfig {
        const entities::DialogInputs lookup;
        const std::string            id;
        const std::string            name;
    };

    struct LabelInputConfig {
        const std::string id;
        const std::string name;
        const std::string title;
        const std::string prefix;
    };

    struct DimensionTableTitle {
        const std::string id;
        const std::string name;
        const std::string label;
        const int         column;
        const int         span;
    };

    struct PanelReferencePointIds {
        const entities::DialogInputs length;
        const entities::DialogInputs width;
        const entities::DialogInputs height;
        const entities::DialogInputs thickness;
        const entities::DialogInputs max;
    };

    using jointTypes = std::vector<std::tuple<entities::JointPatternType, std::vector<entities::AxisFlag>>>;
    using jointPositions = std::map<entities::Position, jointTypes>;

    struct PanelDefaultConfiguration {
        const entities::Panels       id;
        const int                    priority;
        const LabelInputConfig       label;
        const EnabledInputConfig     enable;
        const OverrideInputConfig    override;
        const ThicknessInputConfig   thickness;
        const entities::AxisFlag     orientation;
    };

    struct DimensionTable {
        const std::string                      id;
        const std::string                      name;
        const std::string                      column_ratio;
        const std::vector<DimensionTableTitle>       titles;
        const std::vector<PanelDefaultConfiguration> dimensions;
    };

}

#endif //SILVANUSPRO_INPUTCONFIG_HPP
