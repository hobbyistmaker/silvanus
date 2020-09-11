//
// Created by Hobbyist Maker on 8/11/20.
//

#ifndef SILVANUSPRO_DIVIDERS_HPP
#define SILVANUSPRO_DIVIDERS_HPP

#include "entities/AxisFlag.hpp"
#include "entities/DialogInputs.hpp"
#include "entities/Dimensions.hpp"
#include "entities/Enabled.hpp"
#include "entities/PanelMaxPoint.hpp"
#include "entities/FingerPattern.hpp"
#include "entities/FingerWidth.hpp"
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
#include "entities/PanelMinPoint.hpp"
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
#include <plog/Log.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include <string>
#include <utility>

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
    using entities::PanelPlanes;
    using entities::PanelThicknessInput;
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

            applicationPtr            m_app;
            entt::registry            &m_configuration;
            std::vector<entt::entity> m_dividers;

            int m_length = 1;
            int m_width  = 0;
            int m_height = 0;

            std::string m_length_expr;
            std::string m_height_expr;
            std::string m_width_expr;

            int m_priority = 3;

            double      m_max_offset  = 0.0;
            AxisFlag    m_orientation = AxisFlag::Length;
            std::string m_name_prefix = "Length";

        public:
            Dividers(entt::registry& configuration, applicationPtr& app):
                m_configuration{configuration}, m_app{app} {};

            void create() {
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
                auto pocket_offset = (m_max_offset - divider_thickness * total_panels) / pocket_count;

                auto max_offset_expr = m_length_expr.length() > 0 ? m_length_expr : "";
                max_offset_expr = max_offset_expr.length() == 0 && m_width_expr.length() > 0 ? m_width_expr : max_offset_expr;
                max_offset_expr = max_offset_expr.length() == 0 && m_height_expr.length() > 0 ? m_height_expr : max_offset_expr;

                auto pocket_offset_expr = fmt::format("(({0} - thickness * ({1} + 2)) / ({1} + 1))", max_offset_expr, divider_count);

                for (auto divider_num = 1; divider_num < pocket_count; divider_num++) {
                    auto name = m_name_prefix + " Divider " + std::to_string(divider_num);
                    auto divider_pos = pocket_offset * divider_num + divider_thickness * (divider_num + 1);
                    PLOG_DEBUG << "Creating updated divider joint at " << divider_pos;

                    auto inputs = std::map<AxisFlag, floatSpinnerValueVec>{
                        { AxisFlag::Length, floatSpinnerValueVec{ divider_pos, input_width, input_height } },
                        { AxisFlag::Width, floatSpinnerValueVec{ input_length, divider_pos, input_height } },
                        { AxisFlag::Height, floatSpinnerValueVec{ input_length, input_width, divider_pos } }
                    };
                    auto input_values = inputs[m_orientation];
                    auto divider_length = input_values[0];
                    auto divider_width = input_values[1];
                    auto divider_height = input_values[2];

                    PLOG_DEBUG << "Updating dimension formulas for divider";

                    auto max_height = m_height_expr.length() > 0 ? fmt::format("(({0} * {1}) + (thickness * ({1} + 1)))", pocket_offset_expr, divider_num) : "height";
                    PLOG_DEBUG << "Max Height formula: " << max_height;
                    auto max_length = m_length_expr.length() > 0 ? fmt::format("(({0} * {1}) + (thickness * ({1} + 1)))", pocket_offset_expr, divider_num) : "length";
                    PLOG_DEBUG << "Max Length formula: " << max_length;
                    auto max_width = m_width_expr.length() > 0 ? fmt::format("(({0} * {1}) + (thickness * ({1} + 1)))", pocket_offset_expr, divider_num) : "width";
                    PLOG_DEBUG << "Max Width formula: " << max_width;

                    PLOG_DEBUG << "Length:Width:Height == " << divider_length << ":" << divider_width << ":" << divider_height;
                    auto entity = m_configuration.create();
                    PLOG_DEBUG << "Entity is " << (int)entity;
                    m_configuration.emplace<T>(entity);

                    m_configuration.emplace<FingerPattern>(entity);
                    m_configuration.emplace<InsidePanel>(entity);
                    m_configuration.emplace<JointPattern>(entity);
                    m_configuration.emplace<PanelPlanes>(entity);
                    m_configuration.emplace<PanelPlanesParams>(entity);
                    m_configuration.emplace<PanelMaxPoint>(entity);
                    m_configuration.emplace<PanelMaxParam>(entity);
                    m_configuration.emplace<PanelMinPoint>(entity);
                    m_configuration.emplace<PanelMinParam>(entity);
                    m_configuration.emplace<PanelThickness>(entity);

                    m_configuration.emplace<ThicknessParameter>(entity, "thickness"); // TODO: convert to dynamic thickness
                    m_configuration.emplace<MaxLengthParam>(entity, max_length);
                    m_configuration.emplace<MaxWidthParam>(entity, max_width);
                    m_configuration.emplace<MaxHeightParam>(entity, max_height);

                    m_configuration.emplace<Panel>(entity, name, m_priority, m_orientation, m_length, m_width, m_height);
                    m_configuration.emplace<PanelAxis>(entity, m_length, m_width, m_height);
                    m_configuration.emplace<PanelEnabled>(entity, true);
                    m_configuration.emplace<PanelMaximums>(entity, divider_length, divider_width, divider_height);
                    m_configuration.emplace<PanelPosition>(entity, Position::Inside);
                    m_configuration.emplace<PanelThicknessActive>(entity, default_thickness_control);

                    m_dividers.emplace_back(entity);
                }
            }

            template <class O>
            void addOrientation() {
                for (auto const& entity: m_dividers) {
                    m_configuration.emplace<O>(entity);
                }
            }

            auto setAxis(int length, int width, int height) {
                m_length = length;
                m_width = width;
                m_height = height;
                return *this;
            }

            auto setPriority(int priority) {
                m_priority = priority;
                return *this;
            }

            auto setNamePrefix(std::string prefix) {
                m_name_prefix = std::move(prefix);
                return *this;
            }

            auto setOrientation(AxisFlag orientation) {
                m_orientation = orientation;
                return *this;
            }

            auto setMaxOffset(double offset) {
                m_max_offset = offset;
                return *this;
            }

            auto addMaxHeight(std::string value) -> void {
                m_height_expr = std::move(value);
            }

            auto addMaxLength(std::string value) -> void {
                m_length_expr = std::move(value);
            }

            auto addMaxWidth(std::string value) -> void {
                m_width_expr = std::move(value);
            }

    };

}


#endif //SILVANUSPRO_DIVIDERS_HPP
