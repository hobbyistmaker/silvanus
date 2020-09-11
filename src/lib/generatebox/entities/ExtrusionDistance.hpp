//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_EXTRUSIONDISTANCE_HPP
#define SILVANUSPRO_EXTRUSIONDISTANCE_HPP

#include "entities/Dimensions.hpp"

namespace silvanus::generatebox::entities {

    struct ExtrusionDistance {
        double value;
        std::string expression;
    };

    struct ExtrusionDistanceParam {
        std::string expression;
    };

    struct CompareExtrusionDistance {
        bool operator()(const entities::ExtrusionDistance &a, const entities::ExtrusionDistance &b) const {
            return (a.value < b.value);
        };
    };

}
#endif //SILVANUSPRO_EXTRUSIONDISTANCE_HPP
