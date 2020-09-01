//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/27/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "ParametricRenderer.hpp"

#include "Core/Geometry/Point3D.h"

#include "fusion/FingerCutsPattern.hpp"
#include "fusion/PanelFingerSketch.hpp"
#include "fusion/PanelFeature.hpp"
#include "entities/AxisFlag.hpp"
#include "entities/Enabled.hpp"
#include "entities/InsidePanel.hpp"
#include "entities/JointEnabled.hpp"
#include "entities/JointOrientation.hpp"
#include "entities/JointName.hpp"
#include "entities/JointProfileGroup.hpp"
#include "entities/JoinedPanels.hpp"
#include "entities/OutsidePanel.hpp"
#include "entities/Panel.hpp"
#include "entities/PanelExtrusion.hpp"
#include "entities/PanelProfile.hpp"
#include "entities/PanelGroup.hpp"

#include <map>
#include <set>
#include <string>

#include "plog/Log.h"
#include "FingerPattern.hpp"

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::fusion;
using namespace silvanus::generatebox::render;

using std::map;
using std::unordered_map;

void ParametricRenderer::execute(DefaultModelingOrientations model_orientation, const Ptr<Component>& component) {

    auto yup_planes   = axis_plane_map{
        {AxisFlag::Height, component->xZConstructionPlane()},
        {AxisFlag::Length, component->yZConstructionPlane()},
        {AxisFlag::Width,  component->xYConstructionPlane()}
    };
    auto zup_planes   = axis_plane_map{
        {AxisFlag::Height, component->xYConstructionPlane()},
        {AxisFlag::Length, component->yZConstructionPlane()},
        {AxisFlag::Width,  component->xZConstructionPlane()}
    };
    auto orientations = orientation_plane_map{
        {YUpModelingOrientation, yup_planes},
        {ZUpModelingOrientation, zup_planes}
    };

    using jointProfileSet = std::set<size_t>;
    using panelJointGroup = std::map<jointProfileSet, PanelRenderData>;
    using positionPanelGroup = std::map<Position, panelJointGroup>;
    using profilePositionGroup = std::map<PanelProfile, positionPanelGroup, ComparePanelProfile>;
    using axisProfileGroup = std::map<AxisFlag, profilePositionGroup>;

    auto panel_groups = axisProfileGroup{};

    auto      view = m_registry.view<Enabled, JointProfileGroup, JointEnabled, Panel, PanelGroup, JointGroup, PanelExtrusion, JointOrientation, JointName, JointExtrusion, JointDirection>().proxy();
    for (auto &&[entity, enabled, joint_profile_group, joint_enabled, panel, panel_group, joint_group, panel_extrusion, joint_orientation, joint_name, joint_extrusion, joint_direction]: view) {
        PLOG_DEBUG << panel.name << " enabled == " << (int) enabled.value;
        PLOG_DEBUG << panel.name << " joint to " << joint_name.value << " enabled == " << (int) joint_enabled.value;

        if (!enabled.value || !joint_enabled.value) continue;

        PLOG_DEBUG << "Adding Panel " << panel.name << " with joint to " << joint_name.value << " for direct render";
        PLOG_DEBUG << "Joint direction is " << (int)joint_direction.value;
        PLOG_DEBUG << "Joint thickness is " << joint_group.joint_thickness.value;
        PLOG_DEBUG << "Joint pattern type is " << (int)joint_group.profile.joint_type;

        auto &group = panel_groups[panel_group.orientation][panel_group.profile][panel_group.position][joint_profile_group.hashes];

        group.names.insert(panel_extrusion.name);
        group.panels[panel_group.distance].insert(panel_extrusion);

        if ((joint_group.profile.finger_type == FingerPatternType::None) || (joint_group.profile.joint_type == JointPatternType::None)) {
            continue;
        }

        auto& joined_panel_group = group.joints[joint_group.profile.joint_type][joint_group.profile.joint_direction][joint_orientation.axis][joint_group.profile];
        joined_panel_group.names.insert(joint_name.value);
        joined_panel_group.extrusions.insert(joint_extrusion);
    }

    if (panel_groups.empty()) {
        return;
    }

    for (auto& [axis, axis_data] : panel_groups) {
        for (auto& [profile, profile_data] : axis_data) {
            for (auto& [position, position_data]: profile_data) {
                for (auto& [joint_profile, joint_group]: position_data) {
                    auto const &plane     = orientations[model_orientation][axis];
                    auto const &transform = sketch_transforms[model_orientation][axis];

                    auto timeline  = Ptr<Design>{m_app->activeProduct()}->timeline();
                    auto start_pos = timeline->markerPosition();

                    auto const names  = concat_names(std::vector<std::string>(joint_group.names.begin(), joint_group.names.end()));
                    auto const sketch = PanelProfileSketch(names + " Profile Sketch", plane, transform, profile);

                    for (auto const&[distance, extrusions]: joint_group.panels) {

                        if (extrusions.empty()) { continue; }

                        auto const panels = std::vector<PanelExtrusion>{extrusions.begin(), extrusions.end()};

                        auto const feature = renderSinglePanel(names, sketch, panels[0], model_orientation, joint_group.joints);

                        if (!feature->isValid()) {
                            m_app->userInterface()->messageBox("Extrusion has been invalidated.");
                        }

                        if (panels.size() == 1) { continue; }

                        renderPanelCopies(feature, std::vector<PanelExtrusion>{panels.begin() + 1, panels.end()}, panels[0]);
                    }

                    auto const end_pos = timeline->markerPosition() - 1;
                    if ((end_pos - start_pos) <= 0) { continue; }

                    auto const timeline_group = timeline->timelineGroups()->add(start_pos, end_pos);
                    timeline_group->name(names + " Panel Group");
                }
            }
        }
    }

    m_renders.clear();
}

auto ParametricRenderer::renderSinglePanel(
    const std::string& names,
    const PanelProfileSketch& sketch,
    const PanelExtrusion& data,
    const DefaultModelingOrientations& model_orientation,
    jointPatternTypeMap& joints
) -> Ptr<ExtrudeFeature> {
    auto const extrusion = sketch.extrudeProfile(data.distance, data.offset);

    extrusion->name(data.name + " Panel Extrusion");
    auto const body = extrusion->bodies()->item(0);
    body->name(data.name + " Panel Body");

    auto cuts = renderJointSketches(names, data.distance, model_orientation, extrusion, joints);

    for (auto const& cut_pairs: cuts) {
        for (auto const& cut: cut_pairs) {
            auto const &cut_sketch = cut.sketch;
            auto const &cut_names  = cut.group.names;

            std::vector<Ptr<ExtrudeFeature>> features;

            for (auto const& cut_extrusion: cut.group.extrusions) {
                auto const& cut_feature = cut_sketch.cut(cut_extrusion.offset, cut_extrusion.distance, body);
                if (cut_feature) {
                    auto group = cut.group.names;
                    cut_feature->name(names + " " + concat_names({group.begin(), group.end()}) + " Finger Extrusion");
                    features.emplace_back(cut_feature);
                }
            }

            if ((cut.profile.finger_count <= 1) && (!cut.corner)) {
                continue;
            }

            auto const &product        = m_app->activeProduct();
            auto const &design         = adsk::core::Ptr<Design>{product};
            auto const &root_component = design->rootComponent();

            auto replicator = FingerCutsPattern(m_app, root_component);

            auto const &copy_feature = replicator.copy(model_orientation, features, cut.profile, cut.corner);
            if (copy_feature) {
                copy_feature->name(names + " " + concat_names({cut_names.begin(), cut_names.end()}) + " Finger Pattern");
            }
        }
    }

    return extrusion;
}

void ParametricRenderer::renderPanelCopies(
    const Ptr<ExtrudeFeature>& feature,
    const std::vector<PanelExtrusion>& panels,
    const PanelExtrusion &data
) {
    auto      parent = PanelFeature(feature);
    for (auto &panel : panels) {
        auto extrusion = parent.extrudeCopy(panel.distance, panel.offset, data.offset);
        extrusion->name(panel.name + " Panel Extrusion");
        auto body = extrusion->bodies()->item(0);
        body->name(panel.name + " Panel Body");
    }
}

auto ParametricRenderer::renderJointSketches(
    const std::string& panel_name,
    const ExtrusionDistance& panel_thickness,
    const DefaultModelingOrientations& model_orientation,
    const adsk::core::Ptr<ExtrudeFeature>& extrusion,
    jointPatternTypeMap& joints
) -> std::vector<std::vector<CutProfile>>{
    std::vector<std::vector<CutProfile>> cuts;

    auto name_selector = std::map<JointPatternType, std::string>{
        {JointPatternType::BoxJoint,    "Box Finger"},
        {JointPatternType::LapJoint,    "Lap Finger"},
        {JointPatternType::Trim,        "Trim Finger"},
        {JointPatternType::Tenon,       "Tenon"},
        {JointPatternType::DoubleTenon, "Double Tenon"},
        {JointPatternType::TripleTenon, "Triple Tenon"},
        {JointPatternType::QuadTenon,   "Quad Tenon"}
    };

    for (auto &[joint_type, joint_name]: name_selector){
        for (auto&[joint_direction, direction_data]: joints[joint_type]) {
            for (auto &[joint_orientation, joint_groups]: direction_data) {
                for (auto const&[joint_profile, joint_profile_data]: joint_groups) {
                    cuts.emplace_back(
                        renderJointSketch(panel_name, panel_thickness, model_orientation, extrusion, joint_name, joint_orientation, joint_groups));

                    if (joint_profile.corner_width == 0) continue;

                    cuts.emplace_back(
                        renderCornerJointSketch(panel_name, panel_thickness, model_orientation, extrusion, joint_name + "Corner", joint_orientation, joint_groups));

                }
            }
        }
    }

    return cuts;
}

auto ParametricRenderer::renderJointSketch(
    const std::string& panel_name,
    const ExtrusionDistance &panel_thickness,
    const DefaultModelingOrientations &model_orientation,
    const Ptr<ExtrudeFeature> &extrusion,
    const std::string& sketch_prefix,
    const AxisFlag &joint_orientation,
    const profileRenderGroupMap &joint_groups
) -> std::vector<CutProfile>{
    auto pairs    = std::vector<CutProfile>{};

    for (auto const& [profile, joints]: joint_groups) {
        if ((profile.joint_direction == JointDirectionType::Inverted) && (profile.finger_count < 1)) {
            continue;
        }
        auto const suffix         = " " + concat_names(std::vector<std::string>(joints.names.begin(), joints.names.end())) + " " + sketch_prefix + " Sketch";
        auto const profile_name   = panel_name + suffix;
        auto const pattern_offset = profile.pattern_offset;
        auto const finger_width   = profile.finger_width;

        auto sketch = PanelFingerSketch(
            extrusion,
            face_selectors[model_orientation][joint_orientation],
            Point3D::create(pattern_offset, 0, 0),
            Point3D::create(finger_width, panel_thickness.value, 0),
            profile_name
        );
        pairs.emplace_back(CutProfile{sketch, joints, profile});
    }

    return pairs;
}

auto ParametricRenderer::renderCornerJointSketch(
    const std::string& panel_name,
    const ExtrusionDistance &panel_thickness,
    const DefaultModelingOrientations &model_orientation,
    const Ptr<ExtrudeFeature> &extrusion,
    const std::string& sketch_prefix,
    const AxisFlag &joint_orientation,
    const profileRenderGroupMap &joint_groups
) -> std::vector<CutProfile>{
    auto pairs    = std::vector<CutProfile>{};

    for (auto const& [profile, joints]: joint_groups) {
//        if ((profile.joint_direction == JointDirectionType::Inverted) && (profile.finger_count < 1)) {
//            continue;
//        }
        auto const suffix         = " " + concat_names(std::vector<std::string>(joints.names.begin(), joints.names.end())) + " " + sketch_prefix + " Sketch";
        auto const profile_name   = panel_name + suffix;
        auto const pattern_offset = 0;
        auto const finger_width   = profile.corner_width;

        auto sketch = PanelFingerSketch(
            extrusion,
            face_selectors[model_orientation][joint_orientation],
            Point3D::create(pattern_offset, 0, 0),
            Point3D::create(finger_width, panel_thickness.value, 0),
            profile_name
        );
        pairs.emplace_back(CutProfile{sketch, joints, profile, true});
    }

    return pairs;
}
