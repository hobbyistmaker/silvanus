//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/26/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "SilvanusCore.hpp"

#include "lib/generatebox/render/presentation/DirectRenderer.hpp"
#include "lib/generatebox/render/presentation/ParametricRenderer.hpp"
#include "systems/ConfigureJoints.hpp"
#include "systems/ConfigurePanels.hpp"
#include "entities/ProgressDialogControl.hpp"

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::render;
using namespace silvanus::generatebox::systems;

void SilvanusCore::execute(DefaultModelingOrientations orientation, const Ptr<Component>& component, bool is_parametric)
{
    auto const& product = m_app->activeProduct();
    auto const& design = Ptr<Design>{product};

    auto panel_configurator = ConfigurePanels(m_app, m_registry);
    auto joint_configurator = ConfigureJoints(m_app, m_registry);

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

void SilvanusCore::fast_preview(DefaultModelingOrientations orientation, const Ptr<Component> &component)
{
    auto const& product = m_app->activeProduct();
    auto const& design = Ptr<Design>{product};

    design->designType(DirectDesignType);

    configurePanels();
    configureJoints();

    auto renderer = DirectRenderer(m_app, m_registry);
    renderer.execute(orientation, component);
}

void SilvanusCore::full_preview(DefaultModelingOrientations orientation, const Ptr<Component> &component)
{
    auto const& product = m_app->activeProduct();
    auto const& design = Ptr<Design>{product};

    design->designType(ParametricDesignType);

    configurePanels();
    configureJoints();

    auto renderer = ParametricRenderer(m_app, m_registry);
    renderer.execute(orientation, component);
}

void SilvanusCore::configureJoints() const {
    auto joint_configurator = ConfigureJoints(m_app, m_registry);
    joint_configurator.execute();
}

void SilvanusCore::configurePanels() const {
    auto panel_configurator = ConfigurePanels(m_app, m_registry);
    panel_configurator.execute();
}

