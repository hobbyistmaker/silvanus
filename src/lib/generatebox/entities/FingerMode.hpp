//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_FINGERMODE_HPP
#define SILVANUSPRO_FINGERMODE_HPP

namespace silvanus::generatebox::entities {
    enum class FingerMode {
        Automatic, Constant, None, ConstantAdaptive
    };
}


namespace std {
    template<>
    class hash<silvanus::generatebox::entities::FingerMode> {
        public:
            std::size_t operator()(silvanus::generatebox::entities::FingerMode const& key) const noexcept {
                return (size_t) key;
            }
    };
}

#endif //SILVANUSPRO_FINGERMODE_HPP
