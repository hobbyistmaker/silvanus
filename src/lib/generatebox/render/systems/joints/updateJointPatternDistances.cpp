//
// Created by Hobbyist Maker on 9/8/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "entities/AxisFlag.hpp"
#include "entities/Dimensions.hpp"
#include "entities/PanelMaxPoint.hpp"
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

// TODO: Make this more efficient
void updateJointPatternDistanceExpressions(entt::registry& registry) {
    std::unordered_map<AxisFlag, std::unordered_map<AxisFlag, std::function<std::string(PanelMaxParam)>> > reference_selector = {
        {
            AxisFlag::Length, {{
                                   AxisFlag::Height,
                                   [](PanelMaxParam point) {
                                       return point.width;
                                   }},
                                  {
                                      AxisFlag::Width,
                                      [](PanelMaxParam point) {
                                          return point.height;
                                      }},
                              }},
        {
            AxisFlag::Width,  {{
                                   AxisFlag::Length,
                                   [](PanelMaxParam point) {
                                       return point.height;
                                   }},
                                  {
                                      AxisFlag::Height,
                                      [](PanelMaxParam point) {
                                          return point.length;
                                      }}
                              }},
        {
            AxisFlag::Height, {{
                                   AxisFlag::Length,
                                   [](PanelMaxParam point) {
                                       return point.width;
                                   }},
                                  {
                                      AxisFlag::Width,
                                      [](PanelMaxParam point) {
                                          return point.length;
                                      }},
                              }}};

    PLOG_DEBUG << "Started updateJointPatternDistanceExpressions";
    auto view = registry.view<JointPatternDistanceParam, const OrientationGroup, const PanelMaxParam>().proxy();
    for (auto &&[entity, pattern_distance, orientation, reference]: view) {
        PLOG_DEBUG << "Updating Joint Pattern Distance Expression: " << (int)orientation.finger << ":" << (int)orientation.panel;
        auto const& distance = reference_selector[orientation.panel][orientation.finger](reference);
        pattern_distance.expression = distance;
    }
    PLOG_DEBUG << "Finished updateJointPatternDistanceExpressions";
}

void updateJointPatternDistanceValues(entt::registry &registry) {
    std::unordered_map< AxisFlag, std::unordered_map< AxisFlag, std::function<double(PanelMaxPoint)>> > reference_selector = {
            {
                AxisFlag::Length, {{
                                                 AxisFlag::Height,
                                                 [](PanelMaxPoint point) {
                                                     return point.width;
                                                 }},
                                                {
                                                    AxisFlag::Width,
                                                    [](PanelMaxPoint point) {
                                                        return point.height;
                                                    }},
                                            }},
            {
                AxisFlag::Width,  {{
                                                 AxisFlag::Length,
                                                 [](PanelMaxPoint point) {
                                                     return point.height;
                                                 }},
                                                {
                                                    AxisFlag::Height,
                                                    [](PanelMaxPoint point) {
                                                        return point.length;
                                                    }}
                                            }},
            {
                AxisFlag::Height, {{
                                                 AxisFlag::Length,
                                                 [](PanelMaxPoint point) {
                                                     return point.width;
                                                 }},
                                                {
                                                    AxisFlag::Width,
                                                    [](PanelMaxPoint point) {
                                                        return point.length;
                                                    }},
                                            }}};

    PLOG_DEBUG << "Started updateJointPatternDistanceValues";
    auto view = registry.view<JointPatternDistance, const OrientationGroup, const PanelMaxPoint>().proxy();
    for (auto &&[entity, pattern_distance, orientation, reference]: view) {
        PLOG_DEBUG << "Updating Joint Pattern Distance: " << (int)orientation.finger << ":" << (int)orientation.panel;
        auto const& distance = reference_selector[orientation.panel][orientation.finger](reference);
        pattern_distance.value = distance;
    }
    PLOG_DEBUG << "Finished updateJointPatternDistanceValues";
}

void updateJointPatternDistances(entt::registry& registry) {
    updateJointPatternDistanceValues(registry);
    updateJointPatternDistanceExpressions(registry);
}
