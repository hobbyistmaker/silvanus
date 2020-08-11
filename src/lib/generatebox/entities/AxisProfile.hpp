//
// Created by Hobbyist Maker on 8/13/20.
//

#ifndef SILVANUSPRO_AXISPROFILE_HPP
#define SILVANUSPRO_AXISPROFILE_HPP

#include "Point.hpp"

namespace silvanus::generatebox::entities {

    struct AxisProfile {
        Point start;
        Point end;

        AxisProfile(Point c_start, Point c_end) : start{c_start}, end{c_end} {};

        friend bool operator> (const AxisProfile& a1, const AxisProfile& a2) {
            return (a1.start > a2.start) && (a1.end > a2.end);
        }
        friend bool operator< (const AxisProfile& a1, const AxisProfile& a2) {
            return (a1.start < a2.start) && (a1.end < a2.end);
        }
    };

}
#endif //SILVANUSPRO_AXISPROFILE_HPP
