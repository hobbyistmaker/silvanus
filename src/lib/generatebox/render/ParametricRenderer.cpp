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
#include "entities/JointOrientation.hpp"
#include "entities/JoinedPanels.hpp"
#include "entities/PanelExtrusion.hpp"
#include "entities/PanelProfile.hpp"
#include "entities/PanelGroup.hpp"

#include <map>
#include <set>
#include <string>

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

    auto panel_groups = std::map<AxisFlag, std::map<PanelProfile, std::map<Position, PanelRenderData>, ComparePanelProfile>>{};

    auto const view = m_registry.view<Enabled, PanelGroup, JointGroup, PanelExtrusion, JointOrientation, JoinedPanels>();
    for (auto const entity : view) {
        auto const& panel_group       = view.get<PanelGroup>(entity);
        auto const& panel_extrusion   = view.get<PanelExtrusion>(entity);
        auto const& joint_orientation = view.get<JointOrientation>(entity);
        auto const& joined_panels     = view.get<JoinedPanels>(entity);
        auto const& joint_group       = view.get<JointGroup>(entity);

        auto &group = panel_groups[panel_group.orientation][panel_group.profile][panel_group.position];

        group.names.insert(panel_extrusion.name);
        group.panels[panel_group.distance].insert(panel_extrusion);

        if (joint_group.profile.finger_type != FingerMode::None) {
            for (auto &joined_panel: joined_panels.panels) {
                if (joint_group.profile.joint_type == JointType::Inverse) {
                    auto& joined_panel_group = group.joints.inverse[joint_orientation.axis].outside[joint_group.profile];
                    joined_panel_group.names.insert(joined_panel.extrusion.name);
                    joined_panel_group.extrusions.insert(joined_panel.extrusion);
                } else if (joint_group.profile.joint_type == JointType::Corner) {
                    auto& joined_panel_group = group.joints.corner[joint_orientation.axis].outside[joint_group.profile];
                    joined_panel_group.names.insert(joined_panel.extrusion.name);
                    joined_panel_group.extrusions.insert(joined_panel.extrusion);
                } else {
                    auto& joined_panel_group = group.joints.normal[joint_orientation.axis].outside[joint_group.profile];
                    joined_panel_group.names.insert(joined_panel.extrusion.name);
                    joined_panel_group.extrusions.insert(joined_panel.extrusion);
                }
            }
        }
    }

    for (auto const& [axis, axis_data] : panel_groups) {
        for (auto const& [profile, profile_data] : axis_data) {
            for (auto const& [position, position_data]: profile_data) {
                auto const& plane     = orientations[model_orientation][axis];
                auto const& transform = sketch_transforms[model_orientation][axis];

                auto timeline  = Ptr<Design>{m_app->activeProduct()}->timeline();
                auto start_pos = timeline->markerPosition();

                auto const names  = concat_names(std::vector<std::string>(position_data.names.begin(), position_data.names.end()));
                auto const sketch = PanelProfileSketch(names + " Profile Sketch", plane, transform, profile);

                for (auto const& [distance, extrusions]: position_data.panels) {

                    if (extrusions.empty()) { continue; }

                    auto const panels  = std::vector<PanelExtrusion>{extrusions.begin(), extrusions.end()};

                    auto const feature = renderSinglePanel(names, sketch, panels[0], model_orientation, position_data.joints);

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

    m_renders.clear();
}

auto ParametricRenderer::renderSinglePanel(
    const std::string& names,
    const PanelProfileSketch& sketch,
    const PanelExtrusion& data,
    const DefaultModelingOrientations& model_orientation,
    const JointRenderData& joints
) -> Ptr<ExtrudeFeature> {
    auto const extrusion = sketch.extrudeProfile(data.distance, data.offset);

    extrusion->name(data.name + " Panel Extrusion");
    auto const body = extrusion->bodies()->item(0);
    body->name(data.name + " Panel Body");

    std::vector<std::vector<CutProfile>> cuts;

    renderJointSketches(cuts, names, data.distance, model_orientation, extrusion, joints);

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

            if (cut.profile.finger_count <= 1) {
                continue;
            }

            auto const &product        = m_app->activeProduct();
            auto const &design         = adsk::core::Ptr<Design>{product};
            auto const &root_component = design->rootComponent();

            auto replicator = FingerCutsPattern(m_app, root_component);

            auto const &copy_feature = replicator.copy(model_orientation, features, cut.profile);
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

void ParametricRenderer::renderJointSketches(
    std::vector<std::vector<CutProfile>>& cuts,
    const std::string& panel_name,
    const ExtrusionDistance& panel_thickness,
    const DefaultModelingOrientations& model_orientation,
    const adsk::core::Ptr<ExtrudeFeature>& extrusion,
    const JointRenderData& joints
) {
    for (auto&[joint_orientation, joint_groups]: joints.normal) {
        renderJointSketch(cuts, panel_name, panel_thickness, model_orientation, extrusion, "Finger", joint_orientation, joint_groups);
    }
    for (auto&[joint_orientation, joint_groups]: joints.inverse) {
        renderJointSketch(cuts, panel_name, panel_thickness, model_orientation, extrusion, "Finger", joint_orientation, joint_groups);
    }
    for (auto&[joint_orientation, joint_groups]: joints.corner) {
        renderJointSketch(cuts, panel_name, panel_thickness, model_orientation, extrusion, "Corner", joint_orientation, joint_groups);
    }
}

void ParametricRenderer::renderJointSketch(
    std::vector<std::vector<CutProfile>> &cuts,
    const std::string& panel_name,
    const ExtrusionDistance &panel_thickness,
    const DefaultModelingOrientations &model_orientation,
    const Ptr<ExtrudeFeature> &extrusion,
    const std::string& sketch_prefix,
    const AxisFlag &joint_orientation,
    const JointRenderProfileGroup &joint_groups
) {
    auto pairs    = std::vector<CutProfile>{};
    auto const profiles = joint_groups.outside;

    for (auto const& [profile, joints]: profiles) {
        if ((profile.joint_type == JointType::Inverse) && (profile.finger_count <= 1)) {
            continue;
        }
        auto const suffix          = " " + concat_names(std::vector<std::string>(joints.names.begin(), joints.names.end())) + " " + sketch_prefix + " Sketch";
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
    cuts.emplace_back(pairs);
}
