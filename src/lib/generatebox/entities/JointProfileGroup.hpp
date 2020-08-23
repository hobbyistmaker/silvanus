//
// Created by Hobbyist Maker on 8/31/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_JOINTPROFILEGROUP_HPP
#define SILVANUSPRO_JOINTPROFILEGROUP_HPP

#include "entities/JointProfile.hpp"

#include <set>

namespace silvanus::generatebox::entities {

    struct JointProfileGroup {
        std::set<size_t> hashes;
    };

}

#endif //SILVANUSPRO_JOINTPROFILEGROUP_HPP
