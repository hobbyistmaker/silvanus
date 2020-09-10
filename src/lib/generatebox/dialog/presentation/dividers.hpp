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
#include "entities/Kerf.hpp"
#include "entities/InsidePanel.hpp"
#include "entities/MaxOffset.hpp"
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
#include "entities/MaxOffset.hpp"
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

    using applicationPtr = adsk::core::Ptr<adsk::core::Application>;
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

                    m_configuration.emplace<DialogPanelEnableValue>(entity, true);
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
                }
            }

            template<class M>
            void addMaxOffset() {
                auto offset = m_configuration.ctx<M>().control;

                for (auto const& entity: m_dividers) {
                    m_configuration.emplace<MaxOffsetInput>(entity, offset);
                }
            }
    };

}


#endif //SILVANUSPRO_DIVIDERS_HPP
