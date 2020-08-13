//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/25/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_DIALOGINPUTS_HPP
#define SILVANUSPRO_DIALOGINPUTS_HPP

namespace silvanus::generatebox::entities
{

    enum class DialogInputs {
        TopThickness, BottomThickness, LeftThickness, RightThickness, FrontThickness, BackThickness,
        TopEnable, BottomEnable, LeftEnable, RightEnable, FrontEnable, BackEnable,
        TopOverride, BottomOverride, LeftOverride, RightOverride, FrontOverride, BackOverride,
        Length, Width, Height, Kerf, FingerWidth, Thickness, FullPreview, FastPreview, ModelSelection,
        FullPreviewLabel, FastPreviewLabel, LengthDividerCount, WidthDividerCount, HeightDividerCount,
        FingerMode, DividerLapInput, LengthDividerJointInput, WidthDividerJointInput
    };

}

#endif /* silvanuspro_inputs_hpp */
