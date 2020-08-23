//
// Created by Hobbyist Maker on 8/11/20.
//

#ifndef SILVANUSPRO_POSITION_HPP
#define SILVANUSPRO_POSITION_HPP

namespace silvanus::generatebox::entities {

    enum class Position {
            Inside, Outside
    };
}

namespace std {
    template<>
    class hash<silvanus::generatebox::entities::Position> {
        public:
            std::size_t operator()(silvanus::generatebox::entities::Position const& key) const noexcept {
                return (size_t) key;
            }
    };
}

#endif //SILVANUSPRO_POSITION_HPP
