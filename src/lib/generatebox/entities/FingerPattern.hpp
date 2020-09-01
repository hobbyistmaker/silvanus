//
// Created by Hobbyist Maker on 8/9/20.
//

#ifndef SILVANUSPRO_FINGERPATTERN_HPP
#define SILVANUSPRO_FINGERPATTERN_HPP

#include <Core/UserInterface/DropDownCommandInput.h>

namespace silvanus::generatebox::entities {

    enum class FingerPatternType {
            Automatic, Constant, None, ConstantAdaptive
    };

    struct FingerPattern {
        FingerPatternType value = FingerPatternType::Automatic;
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

namespace std {
    template<>
    class hash<silvanus::generatebox::entities::FingerPatternType> {
        public:
            std::size_t operator()(silvanus::generatebox::entities::FingerPatternType const& key) const noexcept {
                return (size_t) key;
            }
    };
}

#endif //SILVANUSPRO_FINGERPATTERN_HPP
