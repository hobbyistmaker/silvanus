//
// Created by Hobbyist Maker on 8/13/20.
//

#ifndef SILVANUSPRO_POINT_HPP
#define SILVANUSPRO_POINT_HPP

namespace silvanus::generatebox::entities {

    struct Point {
        double x;
        double y;
        double z;

        friend bool operator> (const Point& p1, const Point& p2) {
            return (p1.x > p2.x) && (p1.y > p2.y) && (p1.z > p2.z);
        }
        friend bool operator< (const Point& p1, const Point& p2) {
            return (p1.x < p2.x) && (p1.y < p2.y) && (p1.z < p2.z);
        }
        friend bool operator>= (const Point& p1, const Point& p2) {
            return (p1.x >= p2.x) && (p1.y >= p2.y) && (p1.z >= p2.z);
        }
        friend bool operator<= (const Point& p1, const Point& p2) {
            return (p1.x <= p2.x) && (p1.y <= p2.y) && (p1.z <= p2.z);
        }
    };

}

#endif //SILVANUSPRO_POINT_HPP
