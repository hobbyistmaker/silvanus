//
// Created by Hobbyist Maker on 8/5/20.
//

#include "DirectRenderer.hpp"

#include "entities/Enabled.hpp"
#include "entities/FingerPattern.hpp"
#include "entities/PanelGroup.hpp"
#include "entities/PanelExtrusion.hpp"
#include "entities/JointDirection.hpp"
#include "entities/JointEnabled.hpp"
#include "entities/JointName.hpp"
#include "entities/JointOrientation.hpp"
#include "entities/JointProfileGroup.hpp"
#include "entities/JoinedPanels.hpp"
#include "entities/Panel.hpp"

#include "plog/Log.h"

#include <map>
#include <set>

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::render;

void DirectRenderer::execute(DefaultModelingOrientations model_orientation, const Ptr<Component> &component) {

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

        auto& group = panel_groups[panel_group.orientation][panel_group.profile][panel_group.position][joint_profile_group.hashes]; // Panels with different joints are being grouped together

        group.names.insert(panel_extrusion.name);
        group.panels[panel_group.distance].insert(panel_extrusion);

        if ((joint_group.profile.finger_type == FingerPatternType::None) || (joint_group.profile.joint_type == JointPatternType::None)) {
                continue;
        }

        auto& joined_panel_group = group.joints[joint_group.profile.joint_type][joint_group.profile.joint_direction][joint_orientation.axis][joint_group.profile];
        joined_panel_group.names.insert(joint_name.value);
        joined_panel_group.extrusions.insert(joint_extrusion);

//        auto distance = joint_extrusion.distance
        PLOG_DEBUG << "Panel name: " << panel.name;
        PLOG_DEBUG << "Panel offset: " << panel_extrusion.offset.value;
        PLOG_DEBUG << "Panel extrusion: " << panel_extrusion.name;
        PLOG_DEBUG << "Joint extrusion: " << joint_extrusion.name;
        PLOG_DEBUG << "Joint orientation: " << (int)joint_orientation.axis;
        PLOG_DEBUG << "Joint distance: " << joint_group.profile.pattern_distance;
        PLOG_DEBUG << "Joint extrusion distance: " << joint_extrusion.distance.value;
        PLOG_DEBUG << "Joint profile orientation: " << (int)joint_group.profile.joint_orientation;
        PLOG_DEBUG << "Joint Group panel orientation: " << (int)joint_group.profile.panel_orientation;
        PLOG_DEBUG << "Joint extrusion group names is now size of " << joined_panel_group.names.size();
        PLOG_DEBUG << "Joint extrusion group is now size of " << joined_panel_group.extrusions.size();
        PLOG_DEBUG << "Joint direction: " << (int)joint_group.profile.joint_direction;
        PLOG_DEBUG << "Corner width: " << joint_group.profile.corner_width;
        PLOG_DEBUG << "Corner distance: " << joint_group.profile.corner_distance;
    }

    if (panel_groups.empty()) {
        PLOG_DEBUG << "No panels found to render.";
        return;
    }

    processPanelGroups(model_orientation, panel_groups);
}

void DirectRenderer::processPanelGroups(
    const DefaultModelingOrientations &model_orientation,
    const std::map<AxisFlag, std::map<PanelProfile, std::map<Position, std::map<std::set<size_t>, PanelRenderData>>, ComparePanelProfile>> &panel_groups
) {
    for (auto&[axis, axis_data] : panel_groups) {
        for (auto&[profile, profile_data] : axis_data) {
            for (auto& [position, position_data]: profile_data) {
                for (auto& [joint_profile, joint_group]: position_data) {

                    auto names = concat_names(std::vector<std::string>(joint_group.names.begin(), joint_group.names.end()));

                    for (auto const&[distance, extrusions]: joint_group.panels) {

                        if (extrusions.empty()) { continue; }

                        auto panels = std::vector<PanelExtrusion>{extrusions.begin(), extrusions.end()};

                        auto panel = panels[0];

                        auto center           = center_selector[model_orientation][axis](0, 0, 0, panel.offset.value);
                        auto length_center    = profile.length.value / 2;
                        auto width_center     = profile.width.value / 2;
                        auto thickness_center = panel.distance.value / 2;

                        auto const& vectors          = orientation_selector[model_orientation][axis];
                        auto const& transform_vector = transform_selector[model_orientation][axis](
                            length_center, width_center, thickness_center
                        );
                        auto       length_dir        = vectors[0];
                        auto       width_dir         = vectors[1];

                        auto bounding_box = OrientedBoundingBox3D::create(
                            center,
                            length_dir,
                            width_dir,
                            profile.length.value,
                            profile.width.value,
                            panel.distance.value
                        );
                        auto box          = m_temp_mgr->createBox(bounding_box);

                        auto transform = Matrix3D::create();
                        transform->translation(transform_vector);
                        m_temp_mgr->transform(box, transform);

                        for (auto const& [joint_type, joint_data]: joint_group.joints) {
                            PLOG_DEBUG << "Joint type is " << (int)joint_type;
                            for (auto const& [joint_direction, direction_data]: joint_data) {
                                PLOG_DEBUG << "Joint direction is " << (int)joint_direction;
                                renderNormalJoints(model_orientation, axis, joint_group, panel, box, direction_data);
                            }
                        }

                        auto body = m_bodies->add(box);
                        body->name(panel.name + " Panel Body");

                        for (auto const& copy_panel: std::vector<PanelExtrusion>{panels.begin() + 1, panels.end()}) {
                            auto offset   = copy_panel.offset.value;
                            auto copy_box = m_temp_mgr->copy(box);

                            auto copy_transform = Matrix3D::create();
                            copy_transform->translation(copy_transform_selector[model_orientation][axis](offset - panel.offset.value));
                            m_temp_mgr->transform(copy_box, copy_transform);

                            auto copy_body = m_bodies->add(copy_box);
                            copy_body->name(copy_panel.name + " Panel Body");
                        }

                    }
                }
            }
        }
    }
}

void DirectRenderer::renderNormalJoints(
    const DefaultModelingOrientations &model_orientation,
    AxisFlag axis,
    const PanelRenderData& group_data,
    const PanelExtrusion &panel,
    const Ptr<BRepBody> &box,
    const renderJointTypeMap& joints_group
) {
    for (auto const&[joint_orientation, joint_groups]: joints_group) {
        for (auto const&[joint_profile, joint_profile_data]: joint_groups) {
            renderNormalJoint(model_orientation, axis, panel, box, joint_orientation, joint_profile, joint_profile_data);

            PLOG_DEBUG << "Searching for corner cuts in " << panel.name;
            if (joint_profile.corner_width == 0) continue;
            PLOG_DEBUG << "Found corner cuts in " << panel.name;

            renderCornerJoint(model_orientation, axis, panel, box, joint_orientation, joint_profile, joint_profile_data);
        }
    }
}

void DirectRenderer::renderNormalJoint(
    const DefaultModelingOrientations &model_orientation,
    const AxisFlag &axis,
    const PanelExtrusion &panel,
    const Ptr<BRepBody> &box,
    const AxisFlag &joint_orientation,
    const JointProfile &joint_profile,
    const JointRenderGroup &joint_profile_data
) {
    auto finger_count      = joint_profile.finger_count;
    auto finger_width      = joint_profile.finger_width;
    auto pattern_offset    = joint_profile.pattern_offset;
    auto finger_vectors    = joint_orientation_selector[model_orientation][axis][joint_orientation];
    auto finger_length_dir = finger_vectors[0];
    auto finger_width_dir  = finger_vectors[1];

    PLOG_DEBUG << ">>>>>>>> Rendering joint >>>>>>>>";
    PLOG_DEBUG << "Panel name: " << panel.name;
    PLOG_DEBUG << "Finger count: " << finger_count;

    for (int i = 0; i < finger_count; i++) {
        auto            extrusions = std::vector<JointExtrusion>{
            joint_profile_data.extrusions.begin(), joint_profile_data.extrusions.end()
        };
        for (auto const &joint: extrusions) {
            auto       first_joint              = extrusions[0];
            auto       finger_bounding_box      = OrientedBoundingBox3D::create(
                joint_center_selector[model_orientation][axis][joint_orientation](
                    panel.offset.value, panel.distance.value, joint.offset.value, joint.distance.value
                ),
                finger_length_dir,
                finger_width_dir,
                finger_width,
                joint.distance.value,
                panel.distance.value
            );
            auto       finger_offset            = (i * joint_profile.finger_offset) + pattern_offset;
            auto       finger_box               = m_temp_mgr->createBox(finger_bounding_box);
            auto const &finger_transform_vector = joint_transform_selector[model_orientation][axis][joint_orientation](
                finger_width / 2 + finger_offset, 0, 0
            );
            auto       finger_transform         = Matrix3D::create();
            finger_transform->translation(finger_transform_vector);
            m_temp_mgr->transform(finger_box, finger_transform);
//            auto finger_body = m_bodies->add(finger_box); // Enable for testing
//            finger_body->name(panel.name + " " + joint.name + " Finger Body"); // Enable for testing
            PLOG_DEBUG << ">>>>>>>>>>>>>>>>";
            PLOG_DEBUG << joint.name + " Finger Body";
            PLOG_DEBUG << "Joint distance: " << joint.distance.value;
            PLOG_DEBUG << "Panel distance: " << panel.distance.value;
            PLOG_DEBUG << "Panel axis: " << (int)axis;
            PLOG_DEBUG << "Joint orientation: " << (int)joint_orientation;
            PLOG_DEBUG << "Panel offset: " << panel.offset.value;
            PLOG_DEBUG << "Joint offset: " << joint.offset.value;
            PLOG_DEBUG << "Finger offset: " << finger_offset;
            PLOG_DEBUG << "Finger width: " << finger_width;
            m_temp_mgr->booleanOperation(box, finger_box, DifferenceBooleanType);
        }
    }
    PLOG_DEBUG << "<<<<<<<<<<<<<<<<<<<<<<<<";
}

void DirectRenderer::renderCornerJoint(
    const DefaultModelingOrientations &model_orientation,
    const AxisFlag &axis,
    const PanelExtrusion &panel,
    const Ptr<BRepBody> &box,
    const AxisFlag &joint_orientation,
    const JointProfile &joint_profile,
    const JointRenderGroup &joint_profile_data
) {
    auto finger_count      = 2;
    auto finger_width      = joint_profile.corner_width;
    auto pattern_offset    = 0;
    auto finger_vectors    = joint_orientation_selector[model_orientation][axis][joint_orientation];
    auto finger_length_dir = finger_vectors[0];
    auto finger_width_dir  = finger_vectors[1];

    for (auto i: {0, 1}) {
        auto            extrusions = std::vector<JointExtrusion>{
            joint_profile_data.extrusions.begin(), joint_profile_data.extrusions.end()
        };
        for (auto const &joint: extrusions) {
            auto       first_joint              = extrusions[0];
            auto       finger_bounding_box      = OrientedBoundingBox3D::create(
                joint_center_selector[model_orientation][axis][joint_orientation](
                    panel.offset.value, panel.distance.value, joint.offset.value, joint.distance.value
                ),
                finger_length_dir,
                finger_width_dir,
                finger_width,
                joint.distance.value,
                panel.distance.value
            );
            auto       finger_box               = m_temp_mgr->createBox(finger_bounding_box);
            auto const &finger_transform_vector = joint_transform_selector[model_orientation][axis][joint_orientation](
                finger_width / 2 + joint_profile.corner_distance * i, 0, 0
            );
            auto       finger_transform         = Matrix3D::create();
            finger_transform->translation(finger_transform_vector);
            m_temp_mgr->transform(finger_box, finger_transform);
//            auto finger_body = m_bodies->add(finger_box); // Enable for testing
//            finger_body->name(panel.name + " " + joint.name + " Finger Body"); // Enable for testing
            m_temp_mgr->booleanOperation(box, finger_box, DifferenceBooleanType);
        }
    }
}
