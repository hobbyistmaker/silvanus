//
// Created by Hobbyist Maker on 9/16/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_EXTRUDEFEATURE_HPP
#define SILVANUSPRO_EXTRUDEFEATURE_HPP

#include <Fusion/Features/ExtrudeFeature.h>

namespace silvanus::generatebox::entities {

    struct ExtrudeFeatureDimension {
        adsk::core::Ptr<adsk::fusion::ModelParameter> dimension;
    };
}

#endif //SILVANUSPRO_EXTRUDEFEATURE_HPP
