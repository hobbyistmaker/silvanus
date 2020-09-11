//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_MAXOFFSET_HPP
#define SILVANUSPRO_MAXOFFSET_HPP

#include "DialogInputs.hpp"

namespace silvanus::generatebox::entities {
    struct MaxOffset {
        double value = 0;
    };

    struct MaxOffsetInput
    {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct HeightMaxOffsetInput : public MaxOffsetInput {};
    struct LengthMaxOffsetInput : public MaxOffsetInput {};
    struct WidthMaxOffsetInput : public MaxOffsetInput {};
}

#endif //SILVANUSPRO_MAXOFFSET_HPP
