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
#include "entities/JointDirection.hpp"
#include "entities/JointExtrusion.hpp"
#include "entities/JointName.hpp"
#include "entities/JointPatternDistance.hpp"
#include "entities/JointPatternPosition.hpp"
#include "entities/JointPatternTags.hpp"
#include "entities/JointProfile.hpp"
#include "entities/JointThickness.hpp"
#include "entities/Kerf.hpp"
#include "entities/LengthJointInput.hpp"
#include "entities/OrientationTags.hpp"
#include "entities/OutsidePanel.hpp"
#include "entities/PanelExtrusion.hpp"
#include "entities/PanelGroup.hpp"
#include "entities/PanelId.hpp"
#include "entities/Panel.hpp"
#include "entities/PanelOffsetInput.hpp"
#include "entities/PanelOrientation.hpp"
#include "entities/PanelPosition.hpp"
#include "entities/PanelProfile.hpp"
#include "entities/PanelTag.hpp"
#include "entities/Point.hpp"
#include "entities/StartReferencePoint.hpp"
#include "entities/WidthJointInput.hpp"

#include <map>
#include <vector>

#include "plog/Log.h"

using namespace adsk::core;
using namespace adsk::fusion;
using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::systems;

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
    m_registry.view<ExtrusionDistance, Dimensions>().each([](
        auto& distance, auto const& dimensions
    ){
        PLOG_DEBUG << "Updating extrusion distance";
        distance.value = dimensions.thickness;
    });
}

void ConfigurePanels::updateEndReferencePoints() {
    m_registry.view<EndReferencePoint, Dimensions>().each([](
        auto& reference, auto const &dimensions
    ){
        PLOG_DEBUG << "Updating end reference points";
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
        PLOG_DEBUG << "Updating LengthOrientation panel profile";
        profile.length.value = reference.width.value;
        profile.width.value = reference.height.value;
    });

    auto width_view = m_registry.view<PanelProfile, WidthOrientation, EndReferencePoint>();
    width_view.each([](
        auto& profile, auto const& orientation, auto const& reference
    ){
        PLOG_DEBUG << "Updating WidthOrientation panel profile";
        profile.length.value = reference.length.value;
        profile.width.value = reference.height.value;
    });

    auto height_view = m_registry.view<PanelProfile, HeightOrientation, EndReferencePoint>();
    height_view.each([](
        auto& profile, auto const& orientation, auto const& reference
    ){
        PLOG_DEBUG << "Updating HeightOrientation panel profile";
        profile.length.value = reference.length.value;
        profile.width.value = reference.width.value;
    });

    auto kerf_view = m_registry.view<PanelProfile, Kerf>();
    kerf_view.each([](
        auto& profile, auto const& kerf
    ){
        PLOG_DEBUG << "Adjusting panel profile kerf";
        profile.length.value += kerf.value;
        profile.width.value += kerf.value;
    });
}

void ConfigurePanels::updateStartReferencePoints() {
    auto length_view = m_registry.view<StartReferencePoint, LengthOrientation, EndReferencePoint, ExtrusionDistance>();
    length_view.each([](
        auto& start, auto const& orientation, auto const& end, auto const& extrusion_distance
    ){
        PLOG_DEBUG << "Updating start reference points for length orientation";
        auto length = end.length.value - extrusion_distance.value;

        start.length.value = length;
        start.width = end.width;
        start.height = end.height;
    });

    auto width_view = m_registry.view<StartReferencePoint, WidthOrientation, EndReferencePoint, ExtrusionDistance>();
    width_view.each([](
        auto& start, auto const& orientation, auto const& end, auto const& extrusion_distance
    ){
        PLOG_DEBUG << "Updating start reference points for width orientation";
        auto width = end.width.value - extrusion_distance.value;
        start.width.value = width;
        start.length = end.length;
        start.height = end.height;
    });

    auto height_view = m_registry.view<StartReferencePoint, HeightOrientation, EndReferencePoint, ExtrusionDistance>();
    height_view.each([](
        auto& start, auto const& orientation, auto const& end, auto const& extrusion_distance
    ){
        PLOG_DEBUG << "Updating start reference points for height orientation";

        auto height = end.height.value - extrusion_distance.value;
        start.height.value = height;
        start.length = end.length;
        start.width = end.width;
    });
}

void ConfigurePanels::updatePanelOffsets() {
    m_registry.view<PanelOffset, PanelOffsetInput>().each([](
        auto& offset, auto const& input
    ){
        PLOG_DEBUG << "Updating panel offsets";
        offset.value = input.control->value();
    });

    auto length_view = m_registry.view<PanelOffset, LengthOrientation, EndReferencePoint, ExtrusionDistance>();
    length_view.each([](
        auto& offset, auto const& orientation, auto const& end, auto const& distance
    ){
        PLOG_DEBUG << "Adjusting length orientation panel offsets.";
        offset.value += (end.length.value - distance.value);
    });

    auto width_view = m_registry.view<PanelOffset, WidthOrientation, EndReferencePoint, ExtrusionDistance>();
    width_view.each([](
        auto& offset, auto const& orientation, auto const& end, auto const& distance
    ){
        PLOG_DEBUG << "Adjusting width orientation panel offsets.";
        offset.value += (end.width.value - distance.value);
    });

    auto height_view = m_registry.view<PanelOffset, HeightOrientation, EndReferencePoint, ExtrusionDistance>();
    height_view.each([](
        auto& offset, auto const& orientation, auto const& end, auto const& distance
    ){
        PLOG_DEBUG << "Adjusting height orientation panel offsets.";
        offset.value += (end.height.value - distance.value);
    });

    auto kerf_view = m_registry.view<PanelOffset, OutsidePanel, Kerf>();
    kerf_view.each([](
        auto& offset, auto const& panel, auto const& kerf
    ){
        PLOG_DEBUG << "Adjusting panel offset kerf.";
        if (offset.value == 0) return;
        offset.value += kerf.value;
    });

    m_registry.view<JointPanelOffset, OutsidePanel, Kerf>().each([](
        auto& offset, auto const& panel, auto const& kerf
    ){
        PLOG_DEBUG << "Adjusting joint panel offset kerf.";
        if (offset.value == 0) return;
        offset.value += kerf.value;
    });
}

void ConfigurePanels::addPanelGroups() {
    m_registry.view<Panel, PanelProfile, ExtrusionDistance, PanelPosition>().each([this](
        auto entity, auto const& panel, auto const& profile, auto const& distance, auto const& position
    ){
        PLOG_DEBUG << "Creating panel group";
        m_registry.emplace_or_replace<PanelGroup>(
            entity, panel.orientation, profile, distance, position.value
        );
    });
}

void ConfigurePanels::addPanelExtrusions() {
    m_registry.view<Panel, PanelOffset, ExtrusionDistance>().each([this](
        auto entity, auto const& panel, auto const& offset, auto const& distance
    ){
        PLOG_DEBUG << "Creating panel extrusions";
        m_registry.emplace_or_replace<PanelExtrusion>(
            entity, distance, offset, panel.name
        );
    });
}

void ConfigurePanels::addJointExtrusions()
{
    m_registry.view<JointThickness, JointPanelOffset, JointName>().each([this](
        auto entity, auto const& distance, auto const& offset, auto const& name
    ){
        PLOG_DEBUG << "Adding joint extrusion";
        m_registry.emplace<JointExtrusion>(
            entity, distance.value, offset.value, name.value
        );
    });

    m_registry.view<JointExtrusion, InsidePanel, Kerf>().each([](
        auto& extrusion, auto const& panel, auto const& kerf
    ){
        PLOG_DEBUG << "Adjusting joint extrusion kerf";
        extrusion.distance.value -= kerf.value;
        extrusion.offset.value += kerf.value/2;
    });
}
