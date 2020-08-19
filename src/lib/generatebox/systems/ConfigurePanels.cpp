//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/27/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "ConfigurePanels.hpp"
#include "entities/AxisProfile.hpp"
#include "entities/Dimensions.hpp"
#include "entities/EndReferencePoint.hpp"
#include "entities/HeightJointInput.hpp"
#include "entities/InsidePanel.hpp"
#include "entities/JointExtrusion.hpp"
#include "entities/JointPatternPosition.hpp"
#include "entities/JointPatternTags.hpp"
#include "entities/JointPatternType.hpp"
#include "entities/JointProfile.hpp"
#include "entities/Kerf.hpp"
#include "entities/LengthJointInput.hpp"
#include "entities/OrientationTags.hpp"
#include "entities/OutsidePanel.hpp"
#include "entities/PanelExtrusion.hpp"
#include "entities/PanelGroup.hpp"
#include "entities/PanelName.hpp"
#include "entities/PanelOrientation.hpp"
#include "entities/PanelPosition.hpp"
#include "entities/PanelProfile.hpp"
#include "entities/Point.hpp"
#include "entities/StartReferencePoint.hpp"
#include "entities/WidthJointInput.hpp"

#include <map>
#include <vector>

using namespace adsk::core;
using namespace adsk::fusion;
using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::systems;

void ConfigurePanels::execute() {
    updateLengthJoints();
//    findJoints();
    updateExtrusionDistances();
    updateEndReferencePoints();
    updatePanelProfiles();
    updateStartReferencePoints();
    updatePanelOffsets();

    addPanelGroups();
    addPanelExtrusions();
    addJointExtrusions();
}

void ConfigurePanels::updateLengthJoints() {

    auto full_selector = std::map<int, std::function<void(entt::registry&, entt::entity, JointPatternPosition&, JointProfile&)>>{
        {0, { [](entt::registry& registry, entt::entity entity, JointPatternPosition& pattern, JointProfile& profile){
            pattern.joint_type = JointType::Normal;
            profile.joint_type = JointType::Normal;
            registry.emplace_or_replace<NormalJointPattern>(entity);

        }}},
        {1, { [](entt::registry& registry, entt::entity entity, JointPatternPosition& pattern, JointProfile& profile){
            pattern.joint_type = JointType::BottomLap;
            profile.joint_type = JointType::BottomLap;
            registry.emplace_or_replace<BottomLapJointPattern>(entity);
        }}}
    };
    auto none_selector = std::map<int, std::function<void(entt::registry&, entt::entity, JointPatternPosition&, JointProfile&)>>{
        {0, { [](entt::registry& registry, entt::entity entity, JointPatternPosition& pattern, JointProfile& profile){
            pattern.joint_type = JointType::Normal;
            profile.joint_type = JointType::Normal;
            registry.emplace_or_replace<NormalJointPattern>(entity);

        }}},
        {1, { [](entt::registry& registry, entt::entity entity, JointPatternPosition& pattern, JointProfile& profile){
            pattern.joint_type = JointType::None;
            profile.joint_type = JointType::None;
            registry.emplace_or_replace<NoJointPattern>(entity);
        }}}
    };

    m_registry.view<JointPatternPosition, JointProfile, WidthOrientation, LengthJointInput, PanelName>().each([&, this](
        auto entity, auto& pattern, auto& profile, auto const& panel_orientation, auto const& input, auto const& name
    ){
        full_selector[input.control->selectedItem()->index()](this->m_registry, entity, pattern, profile);
    });
    m_registry.view<JointPatternPosition, JointProfile, HeightOrientation, LengthJointInput, PanelName>().each([&, this](
        auto entity, auto& pattern, auto& profile, auto const& panel_orientation, auto const& input, auto const& name
    ){
        none_selector[input.control->selectedItem()->index()](this->m_registry, entity, pattern, profile);
    });

    m_registry.view<JointPatternPosition, JointProfile, LengthOrientation, WidthJointInput, PanelName>().each([&, this](
        auto entity, auto& pattern, auto& profile, auto const& panel_orientation, auto const& input, auto const& name
    ){
        full_selector[input.control->selectedItem()->index()](this->m_registry, entity, pattern, profile);
    });
    m_registry.view<JointPatternPosition, JointProfile, HeightOrientation, WidthJointInput, PanelName>().each([&, this](
        auto entity, auto& pattern, auto& profile, auto const& panel_orientation, auto const& input, auto const& name
    ){
        none_selector[input.control->selectedItem()->index()](this->m_registry, entity, pattern, profile);
    });

//    m_registry.view<JointWidthOrientation, LengthOrientation, WidthJointInput>().each([&, this](
//        auto entity, auto const& joint_orientation, auto const& panel_orientation, auto const& input
//    ){
//        selector[input.control->selectedItem()->index()](this->m_registry, entity);
//    });
//
}

//void ConfigurePanels::findJoints() {
//
//    auto width_view = m_registry.view<PanelOrientation, PanelName, Dimensions>();
//    for (auto const& entity: width_view) {
//        auto dimensions = width_view.get<Dimensions>(entity);
//        auto name = width_view.get<PanelName>(entity).value;
//        auto orientation = width_view.get<PanelOrientation>(entity).axis;
//
//        auto [first_length, first_width, first_height] = dimensions_selector[orientation](dimensions);
//
//        findWidthJoints(entity, name, first_length, first_width, first_height);
//    }
//}
//
//void ConfigurePanels::findWidthJoints(
//    entt::entity first_panel, std::string first, AxisProfile first_length, AxisProfile first_width, AxisProfile first_height
//) {
//    auto width_view = m_registry.view<PanelOrientation, PanelName, Dimensions>();
//
//    for (auto const& second_panel: width_view) {
//        auto dimensions = width_view.get<Dimensions>(second_panel);
//        auto second = width_view.get<PanelName>(second_panel).value;
//        auto orientation = width_view.get<PanelOrientation>(second_panel).axis;
//
//        auto [second_length, second_width, second_height] = dimensions_selector[orientation](dimensions);
//
//        auto length_overlaps = (first_length.start.x <= second_length.end.x) && (first_length.end.x >= second_length.start.x)
//            && (first_length.end.y >= second_length.start.y) && (first_length.start.y <= second_length.end.y);
//
//        auto width_overlaps = (first_width.start.x <= second_width.end.x) && (first_width.end.x >= second_width.start.x)
//            && (first_width.end.y >= second_width.start.y) && (first_width.start.y <= second_width.end.y);
//
//        auto height_overlaps = (first_height.start.x <= second_height.end.x) && (first_height.end.x >= second_height.start.x)
//            && (first_height.end.y >= second_height.start.y) && (first_height.start.y <= second_height.end.y);
//
//
//        if (length_overlaps && width_overlaps && height_overlaps) {
//            auto overlap_msg = first + "(" + std::to_string(first_length.start.x) + "," + std::to_string(first_length.start.y) + ") panel overlaps with " + second + "(" + std::to_string(second_length.start.x) + "," + std::to_string(second_length.start.y) + ") panel.";
//        }
//    }
//}

void ConfigurePanels::updateExtrusionDistances() {
    auto view = m_registry.view<ExtrusionDistance, Dimensions>();

    view.each([](
        auto& distance, auto const& dimensions
    ){
        distance.value = dimensions.thickness;
    });
}

void ConfigurePanels::updateEndReferencePoints() {
    auto view = m_registry.view<EndReferencePoint, Dimensions>();

    view.each([](
        auto& reference, auto const &dimensions
    ){
        reference.length.value = dimensions.length;
        reference.width.value = dimensions.width;
        reference.height.value = dimensions.height;
    });
}

void ConfigurePanels::updatePanelProfiles() {
    auto length_view = m_registry.view<PanelProfile, LengthOrientation, EndReferencePoint>();
    length_view.each([](
        auto& profile, auto const& orientation, auto const& reference
    ){
        profile.length.value = reference.width.value;
        profile.width.value = reference.height.value;
    });

    auto width_view = m_registry.view<PanelProfile, WidthOrientation, EndReferencePoint>();
    width_view.each([](
        auto& profile, auto const& orientation, auto const& reference
    ){
        profile.length.value = reference.length.value;
        profile.width.value = reference.height.value;
    });

    auto height_view = m_registry.view<PanelProfile, HeightOrientation, EndReferencePoint>();
    height_view.each([](
        auto& profile, auto const& orientation, auto const& reference
    ){
        profile.length.value = reference.length.value;
        profile.width.value = reference.width.value;
    });

    auto kerf_view = m_registry.view<PanelProfile, Kerf>();
    kerf_view.each([](
        auto& profile, auto const& kerf
    ){
        profile.length.value += kerf.value;
        profile.width.value += kerf.value;
    });
}

void ConfigurePanels::updateStartReferencePoints() {
    auto length_view = m_registry.view<StartReferencePoint, LengthOrientation, EndReferencePoint, ExtrusionDistance>();
    length_view.each([](
        auto& start, auto const& orientation, auto const& end, auto const& extrusion_distance
    ){
        auto length = end.length.value - extrusion_distance.value;

        start.length.value = length;
        start.width = end.width;
        start.height = end.height;
    });

    auto width_view = m_registry.view<StartReferencePoint, WidthOrientation, EndReferencePoint, ExtrusionDistance>();
    width_view.each([](
        auto& start, auto const& orientation, auto const& end, auto const& extrusion_distance
    ){
        auto width = end.width.value - extrusion_distance.value;
        start.width.value = width;
        start.length = end.length;
        start.height = end.height;
    });

    auto height_view = m_registry.view<StartReferencePoint, HeightOrientation, EndReferencePoint, ExtrusionDistance>();
    height_view.each([](
        auto& start, auto const& orientation, auto const& end, auto const& extrusion_distance
    ){
        auto height = end.height.value - extrusion_distance.value;
        start.height.value = height;
        start.length = end.length;
        start.width = end.width;
    });
}

void ConfigurePanels::updatePanelOffsets() {
    auto length_view = m_registry.view<PanelOffset, LengthOrientation, EndReferencePoint, ExtrusionDistance>();
    length_view.each([](
        auto& offset, auto const& orientation, auto const& end, auto const& distance
    ){
        offset.value = end.length.value - distance.value;
    });

    auto width_view = m_registry.view<PanelOffset, WidthOrientation, EndReferencePoint, ExtrusionDistance>();
    width_view.each([](
        auto& offset, auto const& orientation, auto const& end, auto const& distance
    ){
        offset.value = end.width.value - distance.value;
    });

    auto height_view = m_registry.view<PanelOffset, HeightOrientation, EndReferencePoint, ExtrusionDistance>();
    height_view.each([](
        auto& offset, auto const& orientation, auto const& end, auto const& distance
    ){
        offset.value = end.height.value - distance.value;
    });

    auto joint_view = m_registry.view<JointPanelOffset, PanelOffset>();
    joint_view.each([](
        auto& joint, auto const& panel
    ){
        joint.value = panel.value;
    });

    auto kerf_view = m_registry.view<PanelOffset, OutsidePanel, Kerf>();
    kerf_view.each([](
        auto& offset, auto const& panel, auto const& kerf
    ){
        if (offset.value == 0) return;
        offset.value += kerf.value;
    });

    m_registry.view<JointPanelOffset, OutsidePanel, Kerf>().each([](
        auto& offset, auto const& panel, auto const& kerf
    ){
        if (offset.value == 0) return;
        offset.value += kerf.value;
    });
}

void ConfigurePanels::addPanelGroups() {
    m_registry.view<PanelOrientation, PanelProfile, ExtrusionDistance, PanelPosition>().each([this](
        auto entity, auto const& orientation, auto const& profile, auto const& distance, auto const& position
    ){
        this->m_registry.emplace_or_replace<PanelGroup>(
            entity, orientation.axis, profile, distance, position.value
        );
    });
}

void ConfigurePanels::addPanelExtrusions() {
    m_registry.view<PanelName, PanelOffset, ExtrusionDistance>().each([this](
        auto entity, auto const& name, auto const& offset, auto const& distance
    ){
        this->m_registry.emplace_or_replace<PanelExtrusion>(
            entity, distance, offset, name.value
        );
    });
}

void ConfigurePanels::addJointExtrusions()
{
    m_registry.view<PanelExtrusion>().each([&, this](
        auto entity, auto const& extrusion
    ){
        this->m_registry.emplace_or_replace<JointExtrusion>(
            entity, extrusion.distance, extrusion.offset, extrusion.name
        );
    });

    m_registry.view<JointExtrusion, InsidePanel, Kerf>().each([this](
        auto& extrusion, auto const& panel, auto const& kerf
    ){
        extrusion.distance.value -= kerf.value;
        extrusion.offset.value += kerf.value/2;
    });
}
