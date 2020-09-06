//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/AxisFlag.hpp"
#include "entities/Dimensions.hpp"
#include "entities/EndReferencePoint.hpp"
#include "entities/JointPatternDistance.hpp"
#include "entities/OrientationGroup.hpp"

#include <algorithm>
#include <unordered_map>

#include <entt/entt.hpp>
#include <plog/Log.h>

using std::max;

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;

void updateJointPatternDistances(entt::registry& registry) {
    std::unordered_map< AxisFlag, std::unordered_map< AxisFlag, std::function<Dimension(EndReferencePoint)>> > reference_selector = {
            {
                AxisFlag::Length, {{
                                                 AxisFlag::Height,
                                                 [](EndReferencePoint point) -> Dimension {
                                                     return point.width;
                                                 }},
                                                {
                                                    AxisFlag::Width,
                                                    [](EndReferencePoint point) -> Dimension {
                                                        return point.height;
                                                    }},
                                            }},
            {
                AxisFlag::Width,  {{
                                                 AxisFlag::Length,
                                                 [](EndReferencePoint point) -> Dimension {
                                                     return point.height;
                                                 }},
                                                {
                                                    AxisFlag::Height,
                                                    [](EndReferencePoint point) -> Dimension {
                                                        return point.length;
                                                    }}
                                            }},
            {
                AxisFlag::Height, {{
                                                 AxisFlag::Length,
                                                 [](EndReferencePoint point) -> Dimension {
                                                     return point.width;
                                                 }},
                                                {
                                                    AxisFlag::Width,
                                                    [](EndReferencePoint point) -> Dimension {
                                                        return point.length;
                                                    }},
                                            }}};

    PLOG_DEBUG << "Started updateJointPatternDistances";
    auto view = registry.view<JointPatternDistance, const OrientationGroup, const EndReferencePoint>().proxy();
    for (auto &&[entity, pattern_distance, orientation, reference]: view) {
        PLOG_DEBUG << "Updating Joint Pattern Distance: " << (int)orientation.finger << ":" << (int)orientation.panel;
        auto const& distance = reference_selector[orientation.panel][orientation.finger](reference);
        pattern_distance.value = distance.value;
    }
    PLOG_DEBUG << "Finished updateJointPatternDistances";
}