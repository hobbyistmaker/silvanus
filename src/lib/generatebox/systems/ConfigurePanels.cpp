//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/27/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "ConfigurePanels.hpp"
#include "entities/PanelName.hpp"
#include "entities/PanelOrientation.hpp"
#include "entities/OrientationTags.hpp"
#include "entities/EndReferencePoint.hpp"
#include "entities/InsidePanel.hpp"
#include "entities/Kerf.hpp"
#include "entities/OutsidePanel.hpp"
#include "entities/PanelExtrusion.hpp"
#include "entities/JointExtrusion.hpp"
#include "entities/Dimensions.hpp"
#include "entities/PanelProfile.hpp"
#include "entities/PanelGroup.hpp"
#include "entities/StartReferencePoint.hpp"
#include "entities/PanelPosition.hpp"

using namespace adsk::core;
using namespace adsk::fusion;
using namespace silvanus::generatebox::systems;
using namespace silvanus::generatebox::entities;

void ConfigurePanels::execute() {
    updateExtrusionDistances();
    updateEndReferencePoints();
    updatePanelProfiles();
    updateStartReferencePoints();
    updatePanelOffsets();

    addPanelGroups();
    addPanelExtrusions();
    addJointExtrusions();
}

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
