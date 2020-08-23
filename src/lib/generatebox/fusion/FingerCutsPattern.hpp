//
// Created by Hobbyist Maker on 8/4/20.
//

#ifndef SILVANUSPRO_FINGERCUTSPATTERN_HPP
#define SILVANUSPRO_FINGERCUTSPATTERN_HPP

#include "entities/AxisFlag.hpp"
#include "entities/JointProfile.hpp"

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include <functional>
#include <map>
#include <stdexcept>

namespace silvanus::generatebox::fusion {

    using axesList = std::function<adsk::core::Ptr<adsk::fusion::ConstructionAxis>()>;
    using fingerOrientationMap = std::map<entities::AxisFlag, axesList>;
    using panelOrientationMap = std::map<entities::AxisFlag, fingerOrientationMap>;
    using orientationMap = std::map<adsk::core::DefaultModelingOrientations, panelOrientationMap>;

    class FingerCutsPattern {

        const adsk::core::Ptr<adsk::core::Application>& m_app;
        const adsk::core::Ptr<adsk::fusion::Component>& m_component;

        panelOrientationMap m_yup_axes_selector = {
            {entities::AxisFlag::Height, {
                {entities::AxisFlag::Width, [this](){ return m_component->xConstructionAxis(); } },
                {entities::AxisFlag::Length, [this](){ return m_component->zConstructionAxis(); } }
            }},
            {entities::AxisFlag::Width, {
                {entities::AxisFlag::Height, [this](){ return m_component->xConstructionAxis(); } },
                {entities::AxisFlag::Length, [this](){ return m_component->yConstructionAxis(); } }
            }},
            {entities::AxisFlag::Length, {
                {entities::AxisFlag::Height, [this](){ return m_component->zConstructionAxis(); } },
                {entities::AxisFlag::Width, [this](){ return m_component->yConstructionAxis(); } }
            }}
        };
        panelOrientationMap m_zup_axes_selector = {
            {entities::AxisFlag::Height, {
                {entities::AxisFlag::Width, [this](){ return m_component->xConstructionAxis(); } },
                {entities::AxisFlag::Length, [this](){ return m_component->yConstructionAxis(); } }
            }},
            {entities::AxisFlag::Width, {
                {entities::AxisFlag::Height, [this](){ return m_component->xConstructionAxis(); } },
                {entities::AxisFlag::Length, [this](){ return m_component->zConstructionAxis(); } }
            }},
            {entities::AxisFlag::Length, {
                {entities::AxisFlag::Height, [this](){ return m_component->yConstructionAxis(); } },
                {entities::AxisFlag::Width, [this](){ return m_component->zConstructionAxis(); } }
            }}
        };

        orientationMap m_axes_selector = {
            {adsk::core::YUpModelingOrientation, m_yup_axes_selector},
            {adsk::core::ZUpModelingOrientation, m_zup_axes_selector}
        };

    public:
        FingerCutsPattern(const adsk::core::Ptr<adsk::core::Application>& app, const adsk::core::Ptr<adsk::fusion::Component>& component)
            : m_component{component}, m_app{app} {};
        [[nodiscard]] adsk::core::Ptr<adsk::fusion::RectangularPatternFeature> copy(
            const adsk::core::DefaultModelingOrientations model_orientation,
            const std::vector<adsk::core::Ptr<adsk::fusion::ExtrudeFeature>>& features,
            const entities::JointProfile& profile,
            const bool corner
        );
    };

}

#endif //SILVANUSPRO_FINGERCUTSPATTERN_HPP
