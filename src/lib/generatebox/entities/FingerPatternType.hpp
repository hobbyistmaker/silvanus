//
// Created by Hobbyist Maker on 8/9/20.
//

#ifndef SILVANUSPRO_FINGERPATTERNTYPE_HPP
#define SILVANUSPRO_FINGERPATTERNTYPE_HPP

#include "entities/FingerMode.hpp"

namespace silvanus::generatebox::entities {

    struct FingerPatternType {
        FingerMode value;
    };

    struct FingerPatternInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct FingerPatternTag { bool value = true; };
    struct AutomaticFingerPatternType : public FingerPatternTag {};
    struct ConstantFingerPatternType : public FingerPatternTag {};
    struct ConstantAdaptiveFingerPatternType : public FingerPatternTag {};
    struct NoFingerPatternType : public FingerPatternTag {};
}

#endif //SILVANUSPRO_FINGERPATTERNTYPE_HPP
