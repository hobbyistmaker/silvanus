//
// Created by Hobbyist Maker on 9/1/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_DIVIDERTAGS_HPP
#define SILVANUSPRO_DIVIDERTAGS_HPP

namespace silvanus::generatebox::entities {

    struct Divider { bool value = true; };

    struct LengthDivider: public Divider {};
    struct WidthDivider: public Divider {};
    struct HeightDivider: public Divider {};

    struct StaticPanel : public Divider {};
    struct LengthDividerJoint : public Divider {};
    struct WidthDividerJoint : public Divider {};
    struct HeightDividerJoint : public Divider {};
    struct DividerJoint : public Divider {};

}

#endif //SILVANUSPRO_DIVIDERTAGS_HPP
