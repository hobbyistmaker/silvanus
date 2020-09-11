//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_THICKNESS_HPP
#define SILVANUSPRO_THICKNESS_HPP

#include "FloatSpinnerCommandInput.h"

namespace silvanus::generatebox::entities {

    struct Thickness {
        double value;
    };

    struct ThicknessInput
    {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct HeightThicknessInput : public ThicknessInput {};
    struct WidthThicknessInput : public ThicknessInput {};
    struct LengthThicknessInput : public ThicknessInput {};

    struct ThicknessParameter {
        std::string name;
        double value;
        std::string unit_type;
    };

    struct PanelThicknessParameter {
        std::string expression;
        std::string unit_type = "cm";
    };

    struct JointThicknessParameter {
        std::string expression;
        std::string unit_type = "cm";
    };
}

#endif //SILVANUSPRO_THICKNESS_HPP
