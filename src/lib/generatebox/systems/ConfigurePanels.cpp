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
    PLOG_DEBUG << "Started updateExtrusionDistances";
    auto view = m_registry.view<ExtrusionDistance, const Dimensions>().proxy();

    for (auto &&[entity, distance, dimensions]: view) {
        PLOG_DEBUG << "Updating extrusion distance to " << std::to_string(dimensions.thickness);
        distance.value = dimensions.thickness;
    }
    PLOG_DEBUG << "Finished updateExtrusionDistances";
}

void ConfigurePanels::updateEndReferencePoints() {
    PLOG_DEBUG << "Started updateEndReferencePoints";
    auto view = m_registry.view<EndReferencePoint, const Dimensions>().proxy();

    for (auto &&[entity, reference, dimensions]: view) {
        PLOG_DEBUG << "Updating end reference points";
        reference.length.value = dimensions.length;
        reference.width.value = dimensions.width;
        reference.height.value = dimensions.height;
    }
    PLOG_DEBUG << "Finished updateEndReferencePoints";
}

void ConfigurePanels::updatePanelProfiles() {
    PLOG_DEBUG << "Started updatePanelProfiles";
    auto length_view = m_registry.view<PanelProfile, const LengthOrientation, const EndReferencePoint>().proxy();
    for (auto &&[entity, profile, orientation, reference]: length_view) {
        PLOG_DEBUG << "Updating LengthOrientation panel profile";
        profile.length.value = reference.width.value;
        profile.width.value = reference.height.value;
    }

    auto width_view = m_registry.view<PanelProfile, const WidthOrientation, const EndReferencePoint>().proxy();
    for (auto &&[entity, profile, orientation, reference]: width_view) {
        PLOG_DEBUG << "Updating WidthOrientation panel profile";
        profile.length.value = reference.length.value;
        profile.width.value = reference.height.value;
    }

    auto height_view = m_registry.view<PanelProfile, const HeightOrientation, const EndReferencePoint>().proxy();
    for (auto &&[entity, profile, orientation, reference]: height_view) {
        PLOG_DEBUG << "Updating HeightOrientation panel profile";
        profile.length.value = reference.length.value;
        profile.width.value = reference.width.value;
    }

    auto kerf_view = m_registry.view<PanelProfile, const Kerf>().proxy();
    for (auto &&[entity, profile, kerf]: kerf_view) {
        PLOG_DEBUG << "Adjusting panel profile kerf";
        profile.length.value += kerf.value;
        profile.width.value += kerf.value;
    }
    PLOG_DEBUG << "Finished updatePanelProfiles";
}

void ConfigurePanels::updateStartReferencePoints() {
    PLOG_DEBUG << "Started updateStartReferencePoints";

    auto length_view = m_registry.view<StartReferencePoint, const LengthOrientation, const EndReferencePoint, const ExtrusionDistance>().proxy();
    for (auto &&[entity, start, orientation, end, extrusion_distance]: length_view) {
        PLOG_DEBUG << "Updating start reference points for length orientation";
        auto length = end.length.value - extrusion_distance.value;

        start.length.value = length;
        start.width = end.width;
        start.height = end.height;
    }

    auto width_view = m_registry.view<StartReferencePoint, const WidthOrientation, const EndReferencePoint, const ExtrusionDistance>().proxy();
    for (auto &&[entity, start, orientation, end, extrusion_distance]: width_view) {
        PLOG_DEBUG << "Updating start reference points for width orientation";
        auto width = end.width.value - extrusion_distance.value;
        start.width.value = width;
        start.length = end.length;
        start.height = end.height;
    }

    auto height_view = m_registry.view<StartReferencePoint, const HeightOrientation, const EndReferencePoint, const ExtrusionDistance>().proxy();
    for (auto &&[entity, start, orientation, end, extrusion_distance]: height_view) {
        PLOG_DEBUG << "Updating start reference points for height orientation";

        auto height = end.height.value - extrusion_distance.value;
        start.height.value = height;
        start.length = end.length;
        start.width = end.width;
    }
    PLOG_DEBUG << "Finished updateStartReferencePoints";
}

void ConfigurePanels::updatePanelOffsets() {
    PLOG_DEBUG << "Started updatePanelOffsets";
    auto offset_view = m_registry.view<PanelOffset, const PanelOffsetInput>().proxy();
    for (auto &&[entity, offset, input]: offset_view) {
        PLOG_DEBUG << "Updating panel offsets";
        offset.value = input.control->value();
    }

    auto length_view = m_registry.view<PanelOffset, const LengthOrientation, const EndReferencePoint, const ExtrusionDistance>().proxy();
    for (auto &&[entity, offset, orientation, end, distance]: length_view) {
        PLOG_DEBUG << "Adjusting length orientation panel offsets.";
        offset.value += (end.length.value - distance.value);
    }

    auto width_view = m_registry.view<PanelOffset, const WidthOrientation, const EndReferencePoint, const ExtrusionDistance>().proxy();
    for (auto &&[entity, offset, orientation, end, distance]: width_view) {
        PLOG_DEBUG << "Adjusting width orientation panel offsets.";
        offset.value += (end.width.value - distance.value);
    }

    auto height_view = m_registry.view<PanelOffset, const HeightOrientation, const EndReferencePoint, const ExtrusionDistance>().proxy();
    for (auto &&[entity, offset, orientation, end, distance]: height_view) {
        PLOG_DEBUG << "Adjusting height orientation panel offsets.";
        offset.value += (end.height.value - distance.value);
    }

    auto kerf_view = m_registry.view<PanelOffset, const OutsidePanel, const Kerf>().proxy();
    for (auto &&[entity, offset, outside, kerf]: kerf_view) {
        PLOG_DEBUG << "Adjusting panel offset kerf.";
        if (offset.value == 0) return;
        offset.value += kerf.value;
    }

    auto kerf_joint_view = m_registry.view<JointPanelOffset, const OutsidePanel, const Kerf>().proxy();
    for (auto &&[entity, offset, outside, kerf]: kerf_joint_view) {
        PLOG_DEBUG << "Adjusting joint panel offset kerf.";
        if (offset.value == 0) return;
        offset.value += kerf.value;
    }
    PLOG_DEBUG << "Finished updatePanelOffsets";
}

void ConfigurePanels::addPanelGroups() {
    PLOG_DEBUG << "Started addPanelGroups";
    auto view = m_registry.view<Panel, PanelProfile, ExtrusionDistance, PanelPosition>().proxy();
    for (auto &&[entity, panel, profile, distance, position]: view) {
        PLOG_DEBUG << "Creating panel group";
        m_registry.emplace_or_replace<PanelGroup>(
            entity, panel.orientation, profile, distance, position.value
        );
    }
    PLOG_DEBUG << "Finished addPanelGroups";
}

void ConfigurePanels::addPanelExtrusions() {
    PLOG_DEBUG << "Started addPanelExtrusions";
    auto view = m_registry.view<Panel, PanelOffset, ExtrusionDistance>().proxy();
    for (auto &&[entity, panel, offset, distance]: view) {
        PLOG_DEBUG << "Creating panel extrusions";
        m_registry.emplace_or_replace<PanelExtrusion>(
            entity, distance, offset, panel.name
        );
    }
    PLOG_DEBUG << "Finished addPanelExtrusions";
}

void ConfigurePanels::addJointExtrusions()
{
    PLOG_DEBUG << "Started addJointExtrusions";
    auto thickness_view = m_registry.view<const JointThickness, const JointPanelOffset, const JointName>().proxy();
    for (auto &&[entity, distance, offset, name]: thickness_view) {
        PLOG_DEBUG << "Adding joint extrusion";
        m_registry.emplace<JointExtrusion>(
            entity, distance.value, offset.value, name.value
        );
    }

    auto extrusion_view = m_registry.view<JointExtrusion, const InsidePanel, const Kerf>().proxy();
    for (auto &&[entity, extrusion, inside, kerf]: extrusion_view) {
        PLOG_DEBUG << "Adjusting joint extrusion kerf";
        extrusion.distance.value -= kerf.value;
        extrusion.offset.value += kerf.value/2;
    }
    PLOG_DEBUG << "Finished addJointExtrusions";
}
