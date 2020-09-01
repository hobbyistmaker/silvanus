//
// Created by Hobbyist Maker on 9/4/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "findJoints.hpp"

auto detectPanelCollisionsImpl(const DialogPanelJoint &first, const DialogPanelJoint &second) -> DialogPanelCollisionPair {
    auto const & first_orientation = first.panel.orientation;
    auto const& first_length       = first.planes.length;
    auto const& first_width        = first.planes.width;
    auto const& first_height       = first.planes.height;

    auto const& orientation        = second.panel.orientation;
    auto const& second_length      = second.planes.length;
    auto const& second_width       = second.planes.width;
    auto const& second_height      = second.planes.height;

    auto length_overlaps = (first_length.min_x <= second_length.max_x) && (first_length.max_x >= second_length.min_x)
                           && (first_length.max_y >= second_length.min_y) && (first_length.min_y <= second_length.max_y);

    PLOG_DEBUG << second.panel.name << " length plane:  (" << second_length.min_x << ", " << second_length.min_y << ") to (" << second_length.max_x << ", "
               << second_length.max_y << ")";
    PLOG_DEBUG << second.panel.name << " width plane :  (" << second_width.min_x << ", " << second_width.min_y << ") to (" << second_width.max_x << ", "
               << second_width.max_y << ")";
    PLOG_DEBUG << second.panel.name << " height plane:  (" << second_height.min_x << ", " << second_height.min_y << ") to (" << second_height.max_x << ", "
               << second_height.max_y << ")";

    auto width_overlaps = (first_width.min_x <= second_width.max_x) && (first_width.max_x >= second_width.min_x)
                          && (first_width.max_y >= second_width.min_y) && (first_width.min_y <= second_width.max_y);

    auto height_overlaps = (first_height.min_x <= second_height.max_x) && (first_height.max_x >= second_height.min_x)
                           && (first_height.max_y >= second_height.min_y) && (first_height.min_y <= second_height.max_y);

    auto collision_detected = length_overlaps && width_overlaps && height_overlaps;

    PLOG_DEBUG << first.panel.name << "(" << first_length.min_x << "," << first_length.min_y << ") panel overlaps with " << second.panel.name << "("
               << second_length.min_x << ","
               << second_length.min_y << ") panel.";

    auto const length_width_joint = first_orientation == AxisFlag::Length && orientation == AxisFlag::Width;
    auto const length_width_pos   = length_width_joint * (second_length.min_x - first_length.min_x);
    auto const length_width_jos   = length_width_joint * (second_length.min_y - first_length.min_y);
    auto const length_width_jd    = length_width_joint * (first_length.max_y - second_length.min_y);
//    auto const length_width_outside = (collision_detected && length_width_joint) && (length_width_pos == 0 || (first_width.max_x >= second_width.min_x && first_width.min_x <= second_width.max_x));
    auto length_width_outside = (collision_detected && length_width_joint) && (first_width.max_x >= second_width.max_x || first_width.min_x <= second_width.min_x);
    PLOG_DEBUG << first.panel.name << " length_width plane is outside " << second.panel.name << " == " << length_width_outside;

    auto const length_height_joint = first_orientation == AxisFlag::Length && orientation == AxisFlag::Height;
    auto const length_height_pos   = length_height_joint * (second_length.min_y - first_length.min_y);
    auto const length_height_jos   = length_height_joint * (second_length.min_x - first_length.min_x);
    auto const length_height_jd    = length_height_joint * (first_length.max_x - second_length.min_x);
//    auto const length_height_outside = (collision_detected && length_height_joint) && (length_height_pos == 0 || (first_length.max_y >= second_length.min_y && first_length.min_y <= second_length.max_y));
    auto const length_height_outside = (collision_detected && length_height_joint) && (first_height.max_x >= second_height.max_x || first_height.min_x <= second_height.min_x);
    PLOG_DEBUG << first.panel.name << " length_height plane is outside " << second.panel.name << " == " << length_height_outside;

    auto const width_length_joint = first_orientation == AxisFlag::Width && orientation == AxisFlag::Length;
    auto const width_length_pos   = width_length_joint * (second_width.min_x - first_width.min_x);
    auto const width_length_jos   = width_length_joint * (second_width.min_y - first_width.min_y);
    auto const width_length_jd    = width_length_joint * (first_width.max_y - second_width.min_y);
//    auto const width_length_outside = (collision_detected && width_length_joint) && (width_length_pos == 0 || (first_width.max_x >= second_width.min_x && first_width.min_x <= second_width.max_x));
    auto const width_length_outside = (collision_detected && width_length_joint) && (first_length.max_x >= second_length.max_x || first_length.min_x <= second_length.min_x);
    PLOG_DEBUG << first.panel.name << " width_length plane is outside " << second.panel.name << " == " << width_length_outside;

    const auto width_height_joint = first_orientation == AxisFlag::Width && orientation == AxisFlag::Height;
    auto const width_height_pos   = width_height_joint * (second_width.min_y - first_width.min_y);
    auto const width_height_jos   = width_height_joint * (second_width.min_x - first_width.min_x);
    auto const width_height_jd    = width_height_joint * (first_width.max_x - second_width.min_x);
//    auto const width_height_outside = (collision_detected && width_height_joint) && (width_height_pos == 0 || (first_width.max_y >= second_width.min_y && first_width.min_y <= second_width.max_y));
    auto const width_height_outside = (collision_detected && width_height_joint) && (first_height.max_x >= second_height.max_x || first_height.min_x <= second_height.min_x);
    PLOG_DEBUG << first.panel.name << " width_height plane is outside " << second.panel.name << " == " << width_height_outside;

    const auto height_width_joint = first_orientation == AxisFlag::Height && orientation == AxisFlag::Width;
    auto const height_width_pos   = height_width_joint * (second_height.min_y - first_height.min_y);
    auto const height_width_jos   = height_width_joint * (second_height.min_x - first_height.min_x);
    auto const height_width_jd    = height_width_joint * (first_height.max_x - second_height.min_x);
//    auto const height_width_outside = (collision_detected && height_width_joint) && (height_width_pos == 0 || (first_height.max_y >= second_height.min_y && first_height.min_y <= second_height.max_y));
    auto const height_width_outside = (collision_detected && height_width_joint) && (first_width.max_x >= second_width.max_x || first_width.min_x <= second_width.min_x);
    PLOG_DEBUG << first.panel.name << " height_width plane is outside " << second.panel.name << " == " << height_width_outside;

    const auto height_length_joint = first_orientation == AxisFlag::Height && orientation == AxisFlag::Length;
    auto const height_length_pos   = height_length_joint * (second_height.min_x - first_height.min_x);
    auto const height_length_jos   = height_length_joint * (second_height.min_y - first_height.min_y);
    auto const height_length_jd    = height_length_joint * (first_height.max_y - second_height.min_y);
    auto const height_length_outside = (collision_detected && height_length_joint) && (first_length.max_x >= second_length.max_x || first_length.min_x <= second_length.min_x);
    PLOG_DEBUG << first.panel.name << " height_length plane is outside " << second.panel.name << " == " << height_length_outside;

    auto panel_offset     = std::max({length_width_pos, length_height_pos, width_length_pos, width_height_pos, height_width_pos, height_length_pos});
    auto joint_offset     = std::max({length_width_jos, length_height_jos, width_length_jos, width_height_jos, height_width_jos, height_length_jos});
    auto joint_distance   = std::max({length_width_jd, length_height_jd, width_length_jd, width_height_jd, height_width_jd, height_length_jd});
    auto first_is_primary = first.panel.priority < second.panel.priority;
    auto ranking_str      = first_is_primary ? " is primary." : " is secondary.";
    auto joint_type       = static_cast<DialogJointPatternType>((int) (first.panel.priority > second.panel.priority));

    auto is_outside = (length_width_outside || length_height_outside || width_length_outside || width_height_outside || height_length_outside || height_width_outside);
    PLOG_DEBUG << second.panel.name << " panel offset from " << first.panel.name << " is " << panel_offset;
    PLOG_DEBUG << second.panel.name << " joint offset from " << first.panel.name << " is " << joint_offset;
    PLOG_DEBUG << second.panel.name << " joint distance for " << first.panel.name << " is " << joint_distance;
    PLOG_DEBUG << first.panel.name << ranking_str;
    PLOG_DEBUG << first.panel.name << " is outside " << second.panel.name << " == " << is_outside;

    return {
        collision_detected,
        first_is_primary,
        static_cast<Position>((int)is_outside),
        joint_type,
        panel_offset,
        joint_offset,
        joint_distance
    };
}
