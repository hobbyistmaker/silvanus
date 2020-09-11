//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_FINGERWIDTH_HPP
#define SILVANUSPRO_FINGERWIDTH_HPP

#include <Core/UserInterface/FloatSpinnerCommandInput.h>
#include <Fusion/FusionAll.h>

namespace silvanus::generatebox::entities {
    struct FingerWidth {
        double value = 0.0;
    };

    struct FingerWidthParam {
        std::string expression;
    };

    struct FingerWidthInput
    {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct FingerWidthDimension {
        adsk::core::Ptr<adsk::fusion::ModelParameter> dimension;
    };

    struct FingerOffsetDimension {
        adsk::core::Ptr<adsk::fusion::ModelParameter> dimension;
    };

    struct FingerCutDimension {
        adsk::core::Ptr<adsk::fusion::ExtrudeFeature> dimension;
    };

    struct FingerCopyFeature {
        adsk::core::Ptr<adsk::fusion::RectangularPatternFeature> feature;
    };

    struct CornerWidthDimension {
        adsk::core::Ptr<adsk::fusion::ModelParameter> dimension;
    };

    struct CornerCutDimension {
        adsk::core::Ptr<adsk::fusion::ExtrudeFeature> dimension;
    };

    struct CornerCopyFeature {
        adsk::core::Ptr<adsk::fusion::RectangularPatternFeature> feature;
    };

}

#endif //SILVANUSPRO_FINGERWIDTH_HPP
