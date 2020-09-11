//
// Created by Hobbyist Maker on 9/18/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_JOINTPANELOFFSET_HPP
#define SILVANUSPRO_JOINTPANELOFFSET_HPP

#include <string>

namespace silvanus::generatebox::entities {

    struct JointPanelOffset {
        double value;
        std::string expression;
    };

    struct JointPanelOffsetParam {
        std::string expression;
    };

}

#endif //SILVANUSPRO_JOINTPANELOFFSET_HPP
