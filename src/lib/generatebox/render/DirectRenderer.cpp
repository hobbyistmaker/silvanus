//
// Created by Hobbyist Maker on 8/5/20.
//

#include "DirectRenderer.hpp"

#include "entities/Enabled.hpp"
#include "entities/PanelGroup.hpp"
#include "entities/PanelExtrusion.hpp"
#include "entities/JointOrientation.hpp"
#include "entities/JoinedPanels.hpp"
#include "entities/OutsidePanel.hpp"
#include "entities/InsidePanel.hpp"

#include <map>

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::render;

void DirectRenderer::execute(DefaultModelingOrientations model_orientation, const Ptr<Component> &component) {

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

    auto      view = m_registry.view<Enabled, OutsidePanel, PanelGroup, JointGroup, PanelExtrusion, JointOrientation, JoinedPanels>();
    for (auto entity : view) {
        auto &panel_group        = view.get<PanelGroup>(entity);
        auto &panel_extrusion    = view.get<PanelExtrusion>(entity);
        auto &joint_orientation  = view.get<JointOrientation>(entity);
        auto &joined_panels      = view.get<JoinedPanels>(entity);
        auto &joint_group = view.get<JointGroup>(entity);

        auto &group = panel_groups[panel_group.orientation][panel_group.profile][panel_group.position];

        group.names.insert(panel_extrusion.name);
        group.panels[panel_group.distance].insert(panel_extrusion);

        if (joint_group.profile.finger_type != FingerMode::None) {
            for (auto &joined_panel: joined_panels.panels) {
                if (joint_group.profile.joint_type == JointType::Inverse) {
                    auto &joined_panel_group = group.joints.inverse[joint_orientation.axis].outside[joint_group.profile];
                    joined_panel_group.names.insert(joined_panel.extrusion.name);
                    joined_panel_group.extrusions.insert(joined_panel.extrusion);
                } else if (joint_group.profile.joint_type == JointType::Corner) {
                    auto &joined_panel_group = group.joints.corner[joint_orientation.axis].outside[joint_group.profile];
                    joined_panel_group.names.insert(joined_panel.extrusion.name);
                    joined_panel_group.extrusions.insert(joined_panel.extrusion);
                } else {
                    auto &joined_panel_group = group.joints.normal[joint_orientation.axis].outside[joint_group.profile];
                    joined_panel_group.names.insert(joined_panel.extrusion.name);
                    joined_panel_group.extrusions.insert(joined_panel.extrusion);
                }
            }
        }
    }

    auto      inside_view = m_registry.view<Enabled, InsidePanel, PanelGroup, JointGroup, PanelExtrusion, JointOrientation, JoinedPanels>();
    for (auto entity : inside_view) {
        auto &panel_group        = inside_view.get<PanelGroup>(entity);
        auto &panel_extrusion    = inside_view.get<PanelExtrusion>(entity);
        auto &joint_orientation  = inside_view.get<JointOrientation>(entity);
        auto &joined_panels      = inside_view.get<JoinedPanels>(entity);
        auto &joint_group        = inside_view.get<JointGroup>(entity);

        auto &group = panel_groups[panel_group.orientation][panel_group.profile][panel_group.position];

        group.names.insert(panel_extrusion.name);
        group.panels[panel_group.distance].insert(panel_extrusion);

        if (joint_group.profile.finger_type != FingerMode::None) {
            for (auto &joined_panel: joined_panels.panels) {
                if (joint_group.profile.joint_type == JointType::Inverse) {
                    auto &joined_panel_group = group.joints.inverse[joint_orientation.axis].inside[joint_group.profile];
                    joined_panel_group.names.insert(joined_panel.extrusion.name);
                    joined_panel_group.extrusions.insert(joined_panel.extrusion);
                } else if (joint_group.profile.joint_type == JointType::Corner) {
                    auto &joined_panel_group = group.joints.corner[joint_orientation.axis].inside[joint_group.profile];
                    joined_panel_group.names.insert(joined_panel.extrusion.name);
                    joined_panel_group.extrusions.insert(joined_panel.extrusion);
                } else {
                    auto &joined_panel_group = group.joints.normal[joint_orientation.axis].inside[joint_group.profile];
                    joined_panel_group.names.insert(joined_panel.extrusion.name);
                    joined_panel_group.extrusions.insert(joined_panel.extrusion);
                }
            }
        }
    }

    for (auto&[axis, axis_data] : panel_groups) {
        for (auto&[profile, profile_data] : axis_data) {
            for (auto& [position, position_data]: profile_data) {
                auto &plane = orientations[model_orientation][axis];

                auto names = concat_names(std::vector<std::string>(position_data.names.begin(), position_data.names.end()));

                for (auto const&[distance, extrusions]: position_data.panels) {

                    if (extrusions.empty()) { continue; }

                    auto panels = std::vector<PanelExtrusion>{extrusions.begin(), extrusions.end()};

                    auto panel = panels[0];

                    auto center           = center_selector[model_orientation][axis](0, 0, 0, panel.offset.value);
                    auto length_center    = profile.length.value / 2;
                    auto width_center     = profile.width.value / 2;
                    auto thickness_center = panel.distance.value / 2;

                    auto const &vectors          = orientation_selector[model_orientation][axis];
                    auto const &transform_vector = transform_selector[model_orientation][axis](
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

                    renderNormalJoints(model_orientation, axis, position_data, panel, box, position_data.joints.normal);
                    renderNormalJoints(model_orientation, axis, position_data, panel, box, position_data.joints.inverse);
                    renderCornerJoints(model_orientation, axis, position_data, panel, box, position_data.joints.corner);

                    auto body = m_bodies->add(box);
                    body->name(panel.name + " Panel Body");

                    for (auto const &copy_panel: std::vector<PanelExtrusion>{panels.begin() + 1, panels.end()}) {
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

void DirectRenderer::renderNormalJoints(
    const DefaultModelingOrientations &model_orientation,
    AxisFlag axis,
    const PanelRenderData& group_data,
    const PanelExtrusion &panel,
    const Ptr<BRepBody> &box,
    const std::map<entities::AxisFlag, JointRenderProfileGroup>& joints_group
) {
    for (auto const&[joint_orientation, joint_groups]: joints_group) {
        for (auto const&[joint_profile, joint_profile_data]: joint_groups.outside) {
            renderNormalJoint(model_orientation, axis, panel, box, joint_orientation, joint_profile, joint_profile_data);
        }
        for (auto const&[joint_profile, joint_profile_data]: joint_groups.inside) {
            renderNormalJoint(model_orientation, axis, panel, box, joint_orientation, joint_profile, joint_profile_data);
        }
    }
}

void DirectRenderer::renderNormalJoint(
    const DefaultModelingOrientations &model_orientation, const AxisFlag &axis, const PanelExtrusion &panel, const Ptr<BRepBody> &box,
    const AxisFlag &joint_orientation, const JointProfile &joint_profile, const JointRenderGroup &joint_profile_data
) {
    auto finger_count      = joint_profile.finger_count;
    auto finger_width      = joint_profile.finger_width;
    auto pattern_offset    = joint_profile.pattern_offset;
    auto finger_vectors    = joint_orientation_selector[model_orientation][axis][joint_orientation];
    auto finger_length_dir = finger_vectors[0];
    auto finger_width_dir  = finger_vectors[1];

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
            auto       finger_offset            = (2 * i * joint_profile.finger_offset) + pattern_offset;
            auto       finger_box               = m_temp_mgr->createBox(finger_bounding_box);
            auto const &finger_transform_vector = joint_transform_selector[model_orientation][axis][joint_orientation](
                finger_width / 2 + finger_offset, 0, 0
            );
            auto       finger_transform         = Matrix3D::create();
            finger_transform->translation(finger_transform_vector);
            m_temp_mgr->transform(finger_box, finger_transform);
//                    auto finger_body = m_bodies->add(finger_box); // Enable for testing
//                    finger_body->name(panel.name + " " + joint.name + " Finger Body"); // Enable for testing
            m_temp_mgr->booleanOperation(box, finger_box, DifferenceBooleanType);
        }
    }
}

void DirectRenderer::renderCornerJoints(
    const DefaultModelingOrientations &model_orientation,
    AxisFlag axis,
    const PanelRenderData& group_data,
    const PanelExtrusion &panel,
    const Ptr<BRepBody> &box,
    const std::map<entities::AxisFlag, JointRenderProfileGroup>& joints_group
) {
    for (auto const&[joint_orientation, joint_groups]: joints_group) {
        for (auto const&[joint_profile, joint_profile_data]: joint_groups.outside) {
            renderCornerJoint(model_orientation, axis, panel, box, joint_orientation, joint_profile, joint_profile_data);
        }
        for (auto const&[joint_profile, joint_profile_data]: joint_groups.inside) {
            renderCornerJoint(model_orientation, axis, panel, box, joint_orientation, joint_profile, joint_profile_data);
        }
    }
}

void DirectRenderer::renderCornerJoint(
    const DefaultModelingOrientations &model_orientation, const AxisFlag &axis, const PanelExtrusion &panel, const Ptr<BRepBody> &box,
    const AxisFlag &joint_orientation, const JointProfile &joint_profile, const JointRenderGroup &joint_profile_data
) {
    auto finger_count      = joint_profile.finger_count;
    auto finger_width      = joint_profile.finger_width;
    auto pattern_offset    = joint_profile.pattern_offset;
    auto finger_vectors    = joint_orientation_selector[model_orientation][axis][joint_orientation];
    auto finger_length_dir = finger_vectors[0];
    auto finger_width_dir  = finger_vectors[1];

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
            auto       finger_box               = m_temp_mgr->createBox(finger_bounding_box);
            auto const &finger_transform_vector = joint_transform_selector[model_orientation][axis][joint_orientation](
                finger_width / 2 + joint_profile.pattern_distance * i, 0, 0
            );
            auto       finger_transform         = Matrix3D::create();
            finger_transform->translation(finger_transform_vector);
            m_temp_mgr->transform(finger_box, finger_transform);
//                    auto finger_body = m_bodies->add(finger_box); // Enable for testing
//                    finger_body->name(panel.name + " " + joint.name + " Finger Body"); // Enable for testing
            m_temp_mgr->booleanOperation(box, finger_box, DifferenceBooleanType);
        }
    }
}
