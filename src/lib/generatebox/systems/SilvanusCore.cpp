//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/26/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "SilvanusCore.hpp"

#include "render/DirectRenderer.hpp"
#include "render/ParametricRenderer.hpp"
#include "systems/ConfigureJoints.hpp"
#include "systems/ConfigurePanels.hpp"
#include "systems/ReadInputs.hpp"

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::render;
using namespace silvanus::generatebox::systems;

void SilvanusCore::execute(DefaultModelingOrientations orientation, const Ptr<Component>& component, bool is_parametric)
{
    auto const& product = m_app->activeProduct();
    auto const& design = Ptr<Design>{product};

    auto input_reader = ReadInputs(m_app, m_registry);
    auto panel_configurator = ConfigurePanels(m_app, m_registry);
    auto joint_configurator = ConfigureJoints(m_app, m_registry);

    input_reader.execute();
    panel_configurator.execute();
    joint_configurator.execute();

    design->designType(is_parametric ? ParametricDesignType : DirectDesignType);
    if (is_parametric) {
        auto renderer = ParametricRenderer(m_app, m_registry);
        renderer.execute(orientation, component);
    } else {
        auto renderer = DirectRenderer(m_app, m_registry);
        renderer.execute(orientation, component);
    }
}

void SilvanusCore::update(DefaultModelingOrientations orientation, Ptr<Component> component)
{
}

void SilvanusCore::fast_preview(DefaultModelingOrientations orientation, const Ptr<Component> &component)
{
    auto const& product = m_app->activeProduct();
    auto const& design = Ptr<Design>{product};

    design->designType(DirectDesignType);

    auto input_reader = ReadInputs(m_app, m_registry);
    auto panel_configurator = ConfigurePanels(m_app, m_registry);
    auto joint_configurator = ConfigureJoints(m_app, m_registry);
    auto renderer = DirectRenderer(m_app, m_registry);

    input_reader.execute();
    panel_configurator.execute();
    joint_configurator.execute();
    renderer.execute(orientation, component);
}

void SilvanusCore::full_preview(DefaultModelingOrientations orientation, const Ptr<Component> &component)
{
    auto const& product = m_app->activeProduct();
    auto const& design = Ptr<Design>{product};

    design->designType(ParametricDesignType);

    auto input_reader = ReadInputs(m_app, m_registry);
    auto panel_configurator = ConfigurePanels(m_app, m_registry);
    auto joint_configurator = ConfigureJoints(m_app, m_registry);
    auto renderer = ParametricRenderer(m_app, m_registry);

    input_reader.execute();
    panel_configurator.execute();
    joint_configurator.execute();
    renderer.execute(orientation, component);
}
