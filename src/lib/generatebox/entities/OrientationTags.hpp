//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_ORIENTATIONTAGS_HPP
#define SILVANUSPRO_ORIENTATIONTAGS_HPP

#include "AxisFlag.hpp"
#include "PanelOrientation.hpp"

namespace silvanus::generatebox::entities {
    struct OrientationTag {
        bool value = true;
    };
    struct JointLengthOrientation : public OrientationTag {};
    struct JointWidthOrientation : public OrientationTag {};
    struct JointHeightOrientation : public OrientationTag {};
    struct HeightOrientation : public OrientationTag {};
    struct LengthOrientation : public OrientationTag {};
    struct WidthOrientation : public OrientationTag {};
}

#endif //SILVANUSPRO_ORIENTATIONTAGS_HPP
