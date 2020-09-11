//
// Created by Hobbyist Maker on 8/2/20.
//

#ifndef SILVANUSPRO_JOINTPATTERNVALUE_HPP
#define SILVANUSPRO_JOINTPATTERNVALUE_HPP

#include <string>

namespace silvanus::generatebox::entities {
    struct JointPatternValues {
        int finger_count;
        double finger_width;
        double finger_offset;
        double pattern_distance;
        double pattern_offset;
        double corner_width;
        double corner_distance;
    };

    struct JointPatternExpressions {
        std::string finger_count;
        std::string finger_width;
        std::string finger_offset;
        std::string pattern_distance;
        std::string pattern_offset;
        std::string corner_width;
        std::string corner_distance;
    };
}

#endif //SILVANUSPRO_JOINTPATTERNVALUE_HPP
