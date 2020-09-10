////
//// Created by Hobbyist Maker on 9/2/20.
//// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
////
//
////#include "presentation/dialogcreatepanels.hpp"
//
//#include "entities/ChildPanels.hpp"
//#include "entities/DialogInputs.hpp"
//#include "entities/Enabled.hpp"
//#include "entities/EnableInput.hpp"
//#include "entities/EndReferencePoint.hpp"
//#include "entities/FingerPattern.hpp"
//#include "entities/FingerWidth.hpp"
//#include "entities/JointDirection.hpp"
//#include "entities/JointEnabled.hpp"
//#include "entities/JointName.hpp"
//#include "entities/JointOrientation.hpp"
//#include "entities/JointPanel.hpp"
//#include "entities/JointPatternDistance.hpp"
//#include "entities/JointPatternPosition.hpp"
//#include "entities/JointPosition.hpp"
//#include "entities/KerfInput.hpp"
//#include "entities/MaxOffset.hpp"
//#include "entities/Panel.hpp"
//#include "entities/PanelDimensionInputs.hpp"
//#include "entities/PanelPosition.hpp"
//#include "entities/PanelProfile.hpp"
//#include "entities/ParentPanel.hpp"
//#include "entities/StartReferencePoint.hpp"
//#include "entities/ThicknessInput.hpp"
//
//#include <entt/entt.hpp>
//#include <plog/Log.h>
//
//using namespace silvanus::generatebox::entities;
//using namespace silvanus::generatebox::dialog;
//
//CreateDialogPanels::CreateDialogPanels(entt::registry &source, entt::registry &destination) : m_source{source}, m_destination{destination} {
//
//}
//
//void CreateDialogPanels::createJoints() {
//    auto kerf = m_source.ctx<DialogKerfInput>().control;
//
//    auto master_view = m_source.view<Enabled, FingerPattern, FingerWidth, DialogJoints, DialogPanelCollisionData, DialogPanels, PanelPosition, JointPosition, JointPattern, entities::JointDirections>().proxy();
//    for (auto &&[entity, enabled, fm, fw, joints, collision_data, panels, pp, jp, pt, directions]: master_view) {
//        auto create_panel = [&, this, finger_mode = fm, finger_width = fw, pattern_type = pt, joint_position = jp, panel_position = pp](
//            const DialogPanelJoint& joint, const entities::JointDirection& direction, const DialogPanelJointData collision, const entt::entity& second_panel
//        ) {
//            auto panel_offset = collision.panel_offset;
//            auto joint_offset = collision.joint_offset;
//            auto joint_distance = collision.distance;
//
//            auto panel = m_destination.create();
//            m_destination.emplace<KerfInput>(panel, kerf);
//            m_destination.emplace<FingerPattern>(panel, finger_mode);
//            m_destination.emplace<FingerWidth>(panel, finger_width);
//
//            m_destination.emplace<JointPattern>(panel, pattern_type);
//            m_destination.emplace<JointDirection>(panel, direction);
//            m_destination.emplace<JointPanelOffset>(panel, panel_offset);
//            m_destination.emplace<JointPatternDistance>(panel, joint_distance);
//            m_destination.emplace<JointPosition>(panel, joint_position.value);
//            m_destination.emplace<PanelPosition>(panel, panel_position.value);
//            m_destination.emplace<JointProfile>(
//                panel, Position::Outside, Position::Outside, JointDirectionType::Normal, JointPatternType::BoxJoint, FingerPatternType::Automatic, 0, 0.0, 0.0, 0.0, 0.0, AxisFlag::Length, AxisFlag::Length
//            );
//            m_destination.emplace<JointPatternPosition>(
//                panel, Position::Outside, AxisFlag::Length, JointPatternType::BoxJoint, AxisFlag::Length, Position::Outside
//            );
//
//            m_destination.emplace<PanelOffset>(panel);
//            m_destination.emplace<Dimensions>(panel);
//            m_destination.emplace<EndReferencePoint>(panel);
//            m_destination.emplace<ExtrusionDistance>(panel);
//            m_destination.emplace<MaxOffset>(panel);
//            m_destination.emplace<PanelProfile>(panel);
//            m_destination.emplace<StartReferencePoint>(panel);
//
//            PLOG_DEBUG << "Adding panel registry entity for " << joint.panel.name;
//            first_index[joint.entity].emplace_back(panel);
//            PLOG_DEBUG << joint.panel.name << " now has " << first_index[joint.entity].size() << " elements.";
//            second_index[second_panel].emplace_back(panel);
//        };
//
//        create_panel(joints.first, directions.first, collision_data.first, joints.second.entity);
//        create_panel(joints.second, directions.second, collision_data.second, joints.first.entity);
//    }
//}
//
//void CreateDialogPanels::populateJointData() {
//    auto process_view = m_source.view<
//        const DialogPanelInputs, const DialogPanel, const PanelDimensions, const DialogPanelThickness, const MaxOffsetInput
//    >().proxy();
//    for (auto &&[entity, inputs, panel_data, dimensions, thickness, max_offset]: process_view) {
//        auto first_panels = first_index[entity];
//        auto second_panels = second_index[entity];
//
//        auto parent_panel = m_destination.create();
//        m_destination.emplace<Panel>(parent_panel, panel_data.name, panel_data.priority, panel_data.orientation);
//        m_destination.emplace<ChildPanels>(parent_panel, first_panels);
//
//        for (auto const &panel: first_panels) {
//            PLOG_DEBUG << "Adding enable, panel and dimension data to panel " << panel_data.name;
//            m_destination.emplace<EnableInput>(panel, inputs.enabled);
//            m_destination.emplace<Panel>(panel, panel_data.name, panel_data.priority, panel_data.orientation);
//            m_destination.emplace<Dimensions>(panel, dimensions.length, dimensions.width, dimensions.height);
//            m_destination.emplace<ThicknessInput>(panel, thickness.control);
//            m_destination.emplace<MaxOffsetInput>(panel, max_offset);
//            m_destination.emplace<ParentPanel>(panel, parent_panel);
//        }
//
//        for (auto const &panel: second_panels) {
//            PLOG_DEBUG << "Adding joint name for " << panel_data.name;
//            m_destination.emplace<JointEnabledInput>(panel, inputs.enabled);
//            m_destination.emplace<JointName>(panel, panel_data.name);
//            m_destination.emplace<JointOrientation>(panel, panel_data.orientation);
//            m_destination.emplace<JointThickness>(panel, thickness.control->value());
//        }
//    }
//}
//
