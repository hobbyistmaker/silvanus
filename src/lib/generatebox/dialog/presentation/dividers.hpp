//
// Created by Hobbyist Maker on 8/11/20.
//

#ifndef SILVANUSPRO_DIVIDERS_HPP
#define SILVANUSPRO_DIVIDERS_HPP

#include "entities/AxisFlag.hpp"
#include "entities/DialogInputs.hpp"
#include "entities/Dimensions.hpp"
#include "entities/Enabled.hpp"
#include "entities/EndReferencePoint.hpp"
#include "entities/FingerPattern.hpp"
#include "entities/FingerWidth.hpp"
#include "entities/FingerWidthInput.hpp"
#include "entities/JointName.hpp"
#include "entities/JointOrientation.hpp"
#include "entities/JointPatternDistance.hpp"
#include "entities/JointPatternPosition.hpp"
#include "entities/JointPatternTags.hpp"
#include "entities/JointDirection.hpp"
#include "entities/JointProfile.hpp"
#include "entities/JointThickness.hpp"
#include "entities/JointPattern.hpp"
#include "entities/KerfInput.hpp"
#include "entities/InsidePanel.hpp"
#include "entities/MaxOffset.hpp"
#include "entities/MaxOffsetInput.hpp"
#include "entities/OrientationGroup.hpp"
#include "entities/OrientationTags.hpp"
#include "entities/Panel.hpp"
#include "entities/PanelPosition.hpp"
#include "entities/PanelProfile.hpp"
#include "entities/PanelType.hpp"
#include "entities/Position.hpp"
#include "entities/StartReferencePoint.hpp"
#include "entities/Thickness.hpp"

#include "entities/DialogInputs.hpp"
#include "entities/InsidePanel.hpp"
#include "entities/MaxOffsetInput.hpp"
#include "entities/PanelDimension.hpp"
#include "entities/PanelOrientation.hpp"
#include "entities/PanelPosition.hpp"
#include "entities/Position.hpp"

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <entt/entt.hpp>

#include <string>

#include "plog/Log.h"

namespace silvanus::generatebox::dialog {


    using controlsDef = std::unordered_map<entities::DialogInputs, adsk::core::Ptr<adsk::core::CommandInput> >;
    using jointsDef = std::map<entities::AxisFlag, std::map<entities::JointPatternType, std::vector<entities::Position>>>;

    using applicationPtr = adsk::core::Ptr<adsk::core::Application>;
    using floatSpinnerPtr = adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput>;
    using floatSpinnerPtrVec = std::vector<floatSpinnerPtr>;
    using floatSpinnerValueVec = std::vector<double>;

    using entities::AxisFlag;
    using entities::DialogFingerMode;
    using entities::DialogFingerWidthInput;
    using entities::DialogHeightInput;
    using entities::DialogJointPatternInput;
    using entities::DialogKerfInput;
    using entities::DialogLengthInput;
    using entities::DialogPanel;
    using entities::DialogPanelPlanes;
    using entities::DialogPanelThickness;
    using entities::DialogThicknessInput;
    using entities::DialogWidthInput;
    using entities::InsidePanel;
    using entities::PanelPosition;
    using entities::Position;
    using entities::PanelDimensions;
    using entities::PanelDimensionInputs;
    using entities::MaxOffsetInput;
    using entities::PanelOrientation;

    template <class T, class U>
    class Dividers {

            applicationPtr m_app;
            entt::registry &m_configuration;
            std::vector<entt::entity> m_dividers;

        public:
            Dividers(entt::registry& configuration, applicationPtr& app):
                m_configuration{configuration}, m_app{app} {};

            void create(
                AxisFlag t_panel_orientation,
                const std::string& name_prefix,
                double max_offset,
                int priority
            ) {
                m_dividers.clear();

                PLOG_DEBUG << "Creating updated dividers";
                auto divider_input = m_configuration.ctx<U>().control;
                auto divider_count = divider_input->value();

                if (divider_count <= 0) return;

                auto finger_mode_control = m_configuration.ctx<DialogFingerMode>().control;
                auto finger_width_control = m_configuration.ctx<DialogFingerWidthInput>().control;
                auto kerf_input_control = m_configuration.ctx<DialogKerfInput>().control;
                auto default_thickness_control = m_configuration.ctx<DialogThicknessInput>().control;

                auto input_length = m_configuration.ctx<DialogLengthInput>().control->value();
                auto input_width = m_configuration.ctx<DialogWidthInput>().control->value();
                auto input_height = m_configuration.ctx<DialogHeightInput>().control->value();
                auto divider_thickness = default_thickness_control->value();

                auto pocket_count = divider_count + 1;
                auto total_panels = divider_count + 2;
                auto pocket_offset = (max_offset - divider_thickness * total_panels) / pocket_count;

                for (auto divider_num = 1; divider_num < pocket_count; divider_num++) {
                    auto name = name_prefix + " Divider " + std::to_string(divider_num);
                    auto divider_pos = pocket_offset * divider_num + divider_thickness * (divider_num + 1);
                    PLOG_DEBUG << "Creating updated divider joint at " << divider_pos;

                    auto inputs = std::map<AxisFlag, floatSpinnerValueVec>{
                        { AxisFlag::Length, floatSpinnerValueVec{ divider_pos, input_width, input_height } },
                        { AxisFlag::Width, floatSpinnerValueVec{ input_length, divider_pos, input_height } },
                        { AxisFlag::Height, floatSpinnerValueVec{ input_length, input_width, divider_pos } }
                    };
                    auto input_values = inputs[t_panel_orientation];
                    auto divider_length = input_values[0];
                    auto divider_width = input_values[1];
                    auto divider_height = input_values[2];

                    PLOG_DEBUG << "Length:Width:Height == " << divider_length << ":" << divider_width << ":" << divider_height;
                    auto entity = m_configuration.create();
                    PLOG_DEBUG << "Entity is " << (int)entity;
                    m_configuration.emplace<T>(entity);

                    m_configuration.emplace<DialogPanelThickness>(entity, default_thickness_control);
                    m_configuration.emplace<InsidePanel>(entity);
                    m_configuration.emplace<Panel>(entity, name, priority, t_panel_orientation);
                    m_configuration.emplace<PanelPosition>(entity, Position::Inside);
                    m_configuration.emplace<DialogPanel>(entity, name, priority, t_panel_orientation);
                    m_configuration.emplace<DialogPanelPlanes>(entity);
                    m_configuration.emplace<PanelDimensions>(entity, divider_length, divider_width, divider_height);

                    m_configuration.emplace<FingerPattern>(entity);
                    m_configuration.emplace<JointPattern>(entity);

                    m_dividers.emplace_back(entity);
                }
            }

            template <class O>
            void addOrientation() {
                for (auto const& entity: m_dividers) {
                    m_configuration.emplace<O>(entity);
//                    m_configuration.emplace<PanelOrientation>(entity, orientation);
                }
            }

            template<class M>
            void addMaxOffset() {
                auto offset = m_configuration.ctx<M>().control;

                for (auto const& entity: m_dividers) {
                    m_configuration.emplace<MaxOffsetInput>(entity, offset);
                }
            }

            template <class D>
            void addDividerInput() {
                for (auto const& entity: m_dividers) {
                    auto const &length_divider_outside_joint = m_configuration.ctx<D>().control;
                    m_configuration.emplace<DialogJointPatternInput>(entity, length_divider_outside_joint);
                }
            }

//            void create(
//                const std::string& name,
//                int divider_count,
//                double max_offset,
//                entities::AxisFlag t_panel_orientation
//            ) {
//                auto old_view = m_registry.view<entities::InsidePanel, T>();
//                m_registry.destroy(old_view.begin(), old_view.end());
//
//                if (divider_count <= 0) return;
//
//                auto finger_mode_control = m_configuration.ctx<entities::DialogFingerMode>().control;
//                auto finger_width_control = m_configuration.ctx<entities::DialogFingerWidthInput>().control;
//                auto kerf_input_control = m_configuration.ctx<entities::DialogKerfInput>().control;
//                auto default_thickness_control = m_configuration.ctx<entities::DialogThicknessInput>().control;
//
//                auto input_length = m_configuration.ctx<entities::DialogLengthInput>().control->value();
//                auto input_width = m_configuration.ctx<entities::DialogWidthInput>().control->value();
//                auto input_height = m_configuration.ctx<entities::DialogHeightInput>().control->value();
//                auto divider_thickness = default_thickness_control->value();
//
//                auto pocket_count = divider_count + 1;
//                auto total_panels = divider_count + 2;
//                auto pocket_offset = (max_offset - divider_thickness * total_panels) / pocket_count;
//
//                for (auto divider_num = 1; divider_num < pocket_count; divider_num++) {
//                    auto divider_pos = pocket_offset * divider_num + divider_thickness * (divider_num + 1);
//
//                    for (auto const& joint_orientation: joint_orientations_map[t_panel_orientation]) {
//                        for (auto & [joint_type, joint_positions]: m_joints[joint_orientation]) {
//                            for (auto & joint_position: joint_positions) {
//                                auto entity = m_registry.create();
//
//                                auto inputs = std::map<entities::AxisFlag, std::vector<double>>{
//                                    { entities::AxisFlag::Length, std::vector<double>{ divider_pos, input_width, input_height } },
//                                    { entities::AxisFlag::Width, std::vector<double>{ input_length, divider_pos, input_height } },
//                                    { entities::AxisFlag::Height, std::vector<double>{ input_length, input_width, divider_pos } }
//                                };
//                                auto input_values = inputs[t_panel_orientation];
//                                auto divider_length = input_values[0];
//                                auto divider_width = input_values[1];
//                                auto divider_height = input_values[2];
//
//                                m_registry.emplace<T>(entity);
//
//                                m_registry.emplace<entities::Dimensions>(entity, divider_length, divider_width, divider_height, divider_thickness);
//
//                                m_registry.emplace<entities::Enabled>(entity);
//                                m_registry.emplace<entities::EndReferencePoint>(entity);
//                                m_registry.emplace<entities::ExtrusionDistance>(entity);
//
//                                m_registry.emplace<entities::FingerPatternInput>(entity, finger_mode_control);
//                                m_registry.emplace<entities::FingerPattern>(entity, entities::FingerPatternType::Automatic);
//                                m_registry.emplace<entities::FingerWidth>(entity);
//                                m_registry.emplace<entities::FingerWidthInput>(entity, finger_width_control);
//
//                                m_registry.emplace<entities::InsidePanel>(entity);
//
//                                m_registry.emplace<entities::JointName>(entity, name + " Divider");
//                                m_registry.emplace<entities::JointOrientation>(entity, joint_orientation);
//                                m_registry.emplace<entities::JointPanelOffset>(entity);
//                                m_registry.emplace<entities::JointPatternDistance>(entity);
//                                m_registry.emplace<entities::JointPatternPosition>(
//                                    entity,
//                                    entities::Position::Inside,
//                                    t_panel_orientation,
//                                    joint_type,
//                                    joint_orientation,
//                                    joint_position
//                                );
//                                m_registry.emplace<entities::JointProfile>(
//                                    entity,
//                                    entities::Position::Inside,
//                                    joint_position,
//                                    joint_type,
//                                    entities::FingerPatternType::Automatic,
//                                    0, 0.0, 0.0, 0.0, 0.0,
//                                    t_panel_orientation,
//                                    joint_orientation
//                                );
//                                m_registry.emplace<entities::JointThickness>(entity);
//
//                                m_registry.emplace<entities::KerfInput>(entity, kerf_input_control);
//
//                                m_registry.emplace<entities::MaxOffset>(entity);
//                                m_registry.emplace<entities::MaxOffsetInput>(entity, t_max_offset, m_controls[t_max_offset]);
//
//                                m_registry.emplace<entities::OrientationGroup>(entity, t_panel_orientation, joint_orientation);
//
//                                m_registry.emplace<entities::Panel>(entity, name + " Divider", 0, t_panel_orientation);
//                                m_registry.emplace<entities::PanelOffset>(entity, divider_pos);
//                                m_registry.emplace<entities::PanelPosition>(entity, entities::Position::Inside);
//                                m_registry.emplace<entities::PanelProfile>(entity);
//
//                                m_registry.emplace<entities::StartReferencePoint>(entity);
//
//                                m_registry.emplace<entities::ThicknessInput>(entity, default_thickness_control);
//
//                                std::unordered_map<entities::AxisFlag, std::function<void(entt::entity)>> joint_orientation_selector = {
//                                    { entities::AxisFlag::Length, [this](auto entity) { m_registry.emplace<entities::JointLengthOrientation>(entity); }},
//                                    { entities::AxisFlag::Width,  [this](auto entity) { m_registry.emplace<entities::JointWidthOrientation>(entity); }},
//                                    { entities::AxisFlag::Height, [this](auto entity) { m_registry.emplace<entities::JointHeightOrientation>(entity); }}
//                                };
//                                joint_orientation_selector[joint_orientation](entity);
//
//                                auto joint_type_selector = std::unordered_map<entities::JointPatternType, std::function<void(entt::entity)>>{
//                                    { entities::JointPatternType::Inverse, [this](entt::entity entity) { m_registry.emplace<entities::InverseJointPattern>(entity); }},
//                                    { entities::JointPatternType::Corner, [this](entt::entity entity) { m_registry.emplace<entities::CornerJointPattern>(entity); }},
//                                    { entities::JointPatternType::BottomLap, [this](entt::entity entity) { m_registry.emplace<entities::LapJointPattern>(entity); }},
//                                    { entities::JointPatternType::TopLap, [this](entt::entity entity) { m_registry.emplace<entities::TopLapJointPattern>(entity); }},
//                                    { entities::JointPatternType::Trim, [this](entt::entity entity) { m_registry.emplace<entities::TrimJointPattern>(entity); }},
//                                    { entities::JointPatternType::Tenon, [this](entt::entity entity) { m_registry.emplace<entities::TenonJointPattern>(entity); }},
//                                    { entities::JointPatternType::Mortise, [this](entt::entity entity) { m_registry.emplace<entities::MortiseJointPattern>(entity); }}
//                                };
//                                joint_type_selector[joint_type](entity);
//
//                                auto joint_pos_selector = std::unordered_map<entities::Position, std::function<void(entities::JointPatternType, entt::entity)>>{
//                                    { entities::Position::Inside, [this](entities::JointPatternType joint_type, entt::entity entity) {
//                                           m_registry.emplace<entities::InsideJointPattern>(entity, joint_type);
//                                        }},
//                                    { entities::Position::Outside, [this](entities::JointPatternType joint_type, entt::entity entity) {
//                                           m_registry.emplace<entities::OutsideJointPattern>(entity, joint_type);
//                                        }}
//                                };
//                                joint_pos_selector[joint_position](joint_type, entity);
//                            }
//                        }
//                    }
//                }
//            }
    };

}


#endif //SILVANUSPRO_DIVIDERS_HPP
