//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_FINGERWIDTHINPUT_HPP
#define SILVANUSPRO_FINGERWIDTHINPUT_HPP

#include <BoolValueCommandInput.h>
#include <FloatSpinnerCommandInput.h>

namespace silvanus::generatebox::entities {
    struct FingerWidthInput
    {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };
}

#endif //SILVANUSPRO_FINGERWIDTHINPUT_HPP
