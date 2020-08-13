//
// Created by James Bearce on 8/11/20.
//

#ifndef SILVANUSPRO_DIVIDERS_HPP
#define SILVANUSPRO_DIVIDERS_HPP

#include "entities/AxisFlag.hpp"
#include "entities/DialogInputs.hpp"
#include "entities/Dimensions.hpp"
#include "entities/Enabled.hpp"
#include "entities/EndReferencePoint.hpp"
#include "entities/FingerMode.hpp"
#include "entities/FingerPatternType.hpp"
#include "entities/FingerWidth.hpp"
#include "entities/FingerWidthInput.hpp"
#include "entities/JointName.hpp"
#include "entities/JointPatternDistance.hpp"
#include "entities/JointPatternTags.hpp"
#include "entities/JointPatternType.hpp"
#include "entities/JointProfile.hpp"
#include "entities/JointThickness.hpp"
#include "entities/JointType.hpp"
#include "entities/KerfInput.hpp"
#include "entities/InsidePanel.hpp"
#include "entities/MaxOffset.hpp"
#include "entities/MaxOffsetInput.hpp"
#include "entities/OrientationGroup.hpp"
#include "entities/OrientationTags.hpp"
#include "entities/PanelName.hpp"
#include "entities/PanelPosition.hpp"
#include "entities/PanelProfile.hpp"
#include "entities/PanelType.hpp"
#include "entities/Position.hpp"
#include "entities/StartReferencePoint.hpp"
#include "entities/ThicknessInput.hpp"

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <entt/entt.hpp>

#include <string>

namespace silvanus::generatebox {


    using controlsDef = std::unordered_map<entities::DialogInputs, adsk::core::Ptr<adsk::core::CommandInput> >;

    template <class T>
    class Dividers {

            adsk::core::Ptr<adsk::core::Application> m_app;
            entt::registry &m_registry;
            controlsDef m_controls;

        public:
            Dividers(entt::registry& registry, adsk::core::Ptr<adsk::core::Application>& app, controlsDef& controls) :
                m_registry{registry}, m_app{app}, m_controls{controls} {};

            void initialize(
                const std::string& name,
                entities::DialogInputs count_id,
                entities::DialogInputs t_max_offset,
                entities::AxisFlag t_panel_orientation
            ) {
                auto old_view = m_registry.view<entities::InsidePanel, T>();
                m_registry.destroy(old_view.begin(), old_view.end());

                auto divider_count = adsk::core::Ptr<adsk::core::IntegerSpinnerCommandInput>{m_controls[count_id]}->value();

                if (divider_count <= 0) return;

                auto input_length = adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput>{m_controls[entities::DialogInputs::Length]}->value();
                auto input_width = adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput>{m_controls[entities::DialogInputs::Width]}->value();
                auto input_height = adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput>{m_controls[entities::DialogInputs::Height]}->value();

                auto max_offset = adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput>{m_controls[t_max_offset]}->value();
                auto divider_thickness = adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput>{m_controls[entities::DialogInputs::Thickness]}->value();
                auto pocket_count = divider_count + 1;
                auto total_panels = divider_count + 2;
                auto pocket_offset = (max_offset - divider_thickness * total_panels) / pocket_count;

                auto finger_orientations_map = std::map<entities::AxisFlag, std::vector<entities::AxisFlag>>{
                    {
                        entities::AxisFlag::Length, {
                            entities::AxisFlag::Height, entities::AxisFlag::Width
                        }
                    },
                    {
                        entities::AxisFlag::Width, {
                            entities::AxisFlag::Height, entities::AxisFlag::Length
                        }
                    },
                    {
                        entities::AxisFlag::Height, {
                            entities::AxisFlag::Length, entities::AxisFlag::Width
                        }
                    }
                };

                for (auto divider_num = 1; divider_num < pocket_count; divider_num++) {
                    auto divider_pos = pocket_offset * divider_num + divider_thickness * (divider_num + 1);

                    for (auto const& joint_type: {entities::JointType::Inverse, entities::JointType::Corner}) {
                        for (auto const& joint_position: {entities::Position::Inside, entities::Position::Outside}) {
                            for (auto const& finger_orientation: finger_orientations_map[t_panel_orientation]) {
                                auto entity =m_registry.create();

                                m_registry.emplace<entities::InsidePanel>(entity);
                                m_registry.emplace<entities::PanelPosition>(entity, entities::Position::Inside);
                                m_registry.emplace<entities::Enabled>(entity);
                                m_registry.emplace<entities::PanelName>(entity, name + " Divider");
                                m_registry.emplace<entities::JointName>(entity, name + " Divider");
                                m_registry.emplace<entities::ThicknessInput>(entity,m_controls[entities::DialogInputs::Thickness]);
                                m_registry.emplace<entities::ExtrusionDistance>(entity);
                                m_registry.emplace<entities::JointThickness>(entity);

                                auto inputs = std::map<entities::AxisFlag, std::vector<double>>{
                                    {
                                        entities::AxisFlag::Length,
                                        std::vector<double>{
                                            divider_pos, input_width, input_height
                                        }
                                    },
                                    {
                                        entities::AxisFlag::Width,
                                        std::vector<double>{
                                            input_length, divider_pos, input_height
                                        }
                                    },
                                    {
                                        entities::AxisFlag::Height,
                                        std::vector<double>{
                                            input_length, input_width, divider_pos
                                        }
                                    }
                                };
                                auto input_values = inputs[t_panel_orientation];
                                auto divider_length = input_values[0];
                                auto divider_width = input_values[1];
                                auto divider_height = input_values[2];

                                m_registry.emplace<entities::Dimensions>(entity, divider_length, divider_width, divider_height, divider_thickness);
                                m_registry.emplace<entities::KerfInput>(entity,m_controls[entities::DialogInputs::Kerf]);
                                m_registry.emplace<entities::FingerPatternInput>(entity,m_controls[entities::DialogInputs::FingerMode]);
                                m_registry.emplace<entities::FingerPatternType>(entity, entities::FingerMode::Automatic);
                                m_registry.emplace<entities::FingerWidthInput>(entity,m_controls[entities::DialogInputs::FingerWidth]);
                                m_registry.emplace<entities::FingerWidth>(entity);
                                m_registry.emplace<entities::PanelOrientation>(entity, t_panel_orientation);
                                m_registry.emplace<T>(entity);
                                m_registry.emplace<entities::MaxOffsetInput>(entity, t_max_offset,m_controls[t_max_offset]);
                                m_registry.emplace<entities::MaxOffset>(entity);
                                m_registry.emplace<entities::PanelOffset>(entity, divider_pos);
                                m_registry.emplace<entities::JointPanelOffset>(entity);
                                m_registry.emplace<entities::EndReferencePoint>(entity);
                                m_registry.emplace<entities::StartReferencePoint>(entity);
                                m_registry.emplace<entities::PanelProfile>(entity);
                                m_registry.emplace<entities::JointOrientation>(entity, finger_orientation);
                                m_registry.emplace<entities::JointPatternPosition>(
                                   entity,
                                   entities::Position::Inside,
                                   t_panel_orientation,
                                   joint_type,
                                   finger_orientation,
                                   joint_position
                                );
                                m_registry.emplace<entities::OrientationGroup>(entity, t_panel_orientation, finger_orientation);
                                m_registry.emplace<entities::JointPatternDistance>(entity);
                                m_registry.emplace<entities::JointProfile>(
                                    entity,
                                    entities::PanelType::Inside,
                                    joint_type,
                                    joint_position,
                                    entities::FingerMode::Automatic,
                                    finger_orientation,
                                    t_panel_orientation
                                );

                                std::unordered_map<entities::AxisFlag, std::function<void(entt::entity)>> finger_orientation_selector = {
                                    {
                                        entities::AxisFlag::Length,  [this](auto entity) {
                                       m_registry.emplace<entities::JointLengthOrientation>(entity);
                                    }},
                                    {
                                        entities::AxisFlag::Width,  [this](auto entity) {
                                       m_registry.emplace<entities::JointWidthOrientation>(entity);
                                    }},
                                    {
                                        entities::AxisFlag::Height, [this](auto entity) {
                                       m_registry.emplace<entities::JointHeightOrientation>(entity);
                                    }}
                                };
                                finger_orientation_selector[finger_orientation](entity);

                                auto joint_type_selector = std::unordered_map<entities::JointType, std::function<void(entt::entity)>>{
                                    {
                                        entities::JointType::Inverse,
                                        [this](entt::entity entity) {
                                           m_registry.emplace<entities::InverseJointPattern>(entity);
                                        }
                                    },
                                    {
                                        entities::JointType::Corner,
                                        [this](entt::entity entity) {
                                           m_registry.emplace<entities::CornerJointPattern>(entity);
                                        }
                                    }
                                };
                                joint_type_selector[joint_type](entity);

                                auto joint_pos_selector = std::unordered_map<entities::Position, std::function<void(entt::entity)>>{
                                    {
                                        entities::Position::Inside,
                                        [joint_type, this](entt::entity entity) {
                                           m_registry.emplace<entities::InsideJointPattern>(entity, joint_type);
                                        }
                                    },
                                    {
                                        entities::Position::Outside,
                                        [joint_type, this](entt::entity entity) {
                                           m_registry.emplace<entities::OutsideJointPattern>(entity, joint_type);
                                        }
                                    }
                                };
                                joint_pos_selector[joint_position](entity);
                            }
                        }
                    }
                }
            }
    };

}


#endif //SILVANUSPRO_DIVIDERS_HPP
