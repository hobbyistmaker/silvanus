//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_AXISFLAG_HPP
#define SILVANUSPRO_AXISFLAG_HPP

#include <memory>

namespace silvanus::generatebox::entities {
    enum class AxisFlag {
        Length, Width, Height
    };
}

namespace std {
    template<>
    class hash<silvanus::generatebox::entities::AxisFlag> {
        public:
            std::size_t operator()(silvanus::generatebox::entities::AxisFlag const& key) const noexcept {
                return (size_t) key;
            }
    };
}

#endif //SILVANUSPRO_AXISFLAG_HPP
