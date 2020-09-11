//
// Created by Hobbyist Maker on 9/17/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_EXPRESSIONPARAMETERMAP_HPP
#define SILVANUSPRO_EXPRESSIONPARAMETERMAP_HPP

#include <map>
#include <string>

namespace silvanus::generatebox::entities {

    struct ExpressionParameterMap {
        std::map<std::string, std::string> parameters;
    };

}

#endif //SILVANUSPRO_EXPRESSIONPARAMETERMAP_HPP
