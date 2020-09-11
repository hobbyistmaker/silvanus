//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_PARENTPANEL_HPP
#define SILVANUSPRO_PARENTPANEL_HPP

#include "entities/AxisFlag.hpp"

#include <entt/entt.hpp>

#include <string>

namespace silvanus::generatebox::entities {
    struct ParentPanel
    {
        entt::entity id;
        std::string  name;
        AxisFlag     orientation;
    };
}

#endif //SILVANUSPRO_PARENTPANEL_HPP
