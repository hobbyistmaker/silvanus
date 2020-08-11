//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/27/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_PARAMETRICRENDERER_HPP
#define SILVANUSPRO_PARAMETRICRENDERER_HPP

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <entt/entt.hpp>

#include "rendersupport.hpp"

#include "render/Renderer.hpp"
#include "fusion/PanelFingerSketch.hpp"
#include "fusion/PanelProfileSketch.hpp"
#include "entities/AxisFlag.hpp"
#include "entities/Dimensions.hpp"
#include "entities/JointExtrusion.hpp"
#include "entities/JointProfile.hpp"
#include "entities/JointThickness.hpp"
#include "entities/PanelExtrusion.hpp"
#include "entities/PanelProfile.hpp"

#include <map>
#include <set>
#include <string>
#include <utility>

namespace silvanus::generatebox::render {

    class ParametricRenderer : public Renderer {

            axisFaceSelector yup_faces = {
                {
                    entities::AxisFlag::Width,  [](const adsk::core::Ptr<adsk::fusion::BRepBody> &body) {
                    return findBodyFace(
                        body, [](const adsk::core::Ptr<adsk::fusion::BRepFace> &lhs, const adsk::core::Ptr<adsk::fusion::BRepFace> &rhs) {
                            return lhs->centroid()->z() < rhs->centroid()->z();
                        }
                    );
                }},
                {
                    entities::AxisFlag::Length, [](const adsk::core::Ptr<adsk::fusion::BRepBody> &body) {
                    return findBodyFace(
                        body, [](const adsk::core::Ptr<adsk::fusion::BRepFace> &lhs, const adsk::core::Ptr<adsk::fusion::BRepFace> &rhs) {
                            return lhs->centroid()->x() < rhs->centroid()->x();
                        }
                    );
                }},
                {
                    entities::AxisFlag::Height, [](const adsk::core::Ptr<adsk::fusion::BRepBody> &body) {
                    return findBodyFace(
                        body, [](const adsk::core::Ptr<adsk::fusion::BRepFace> &lhs, const adsk::core::Ptr<adsk::fusion::BRepFace> &rhs) {
                            return lhs->centroid()->y() < rhs->centroid()->y();
                        }
                    );
                }}
            };

            axisFaceSelector zup_faces = {
                {
                    entities::AxisFlag::Width,  [](const adsk::core::Ptr<adsk::fusion::BRepBody> &body) {
                    return findBodyFace(
                        body, [](const adsk::core::Ptr<adsk::fusion::BRepFace> &lhs, const adsk::core::Ptr<adsk::fusion::BRepFace> &rhs) {
                            return lhs->centroid()->y() < rhs->centroid()->y();
                        }
                    );
                }},
                {
                    entities::AxisFlag::Length, [](const adsk::core::Ptr<adsk::fusion::BRepBody> &body) {
                    return findBodyFace(
                        body, [](const adsk::core::Ptr<adsk::fusion::BRepFace> &lhs, const adsk::core::Ptr<adsk::fusion::BRepFace> &rhs) {
                            return lhs->centroid()->x() < rhs->centroid()->x();
                        }
                    );
                }},
                {
                    entities::AxisFlag::Height, [](const adsk::core::Ptr<adsk::fusion::BRepBody> &body) {
                    return findBodyFace(
                        body, [](const adsk::core::Ptr<adsk::fusion::BRepFace> &lhs, const adsk::core::Ptr<adsk::fusion::BRepFace> &rhs) {
                            return lhs->centroid()->z() < rhs->centroid()->z();
                        }
                    );
                }}
            };

            static adsk::core::Ptr<adsk::fusion::BRepFace> findBodyFace(
                const adsk::core::Ptr<adsk::fusion::BRepBody> &body,
                std::function<bool(const adsk::core::Ptr<adsk::fusion::BRepFace> &lhs, const adsk::core::Ptr<adsk::fusion::BRepFace> &rhs)> func
            ) {
                auto faces = std::vector<adsk::core::Ptr<adsk::fusion::BRepFace>>{};
                for (auto const &face: body->faces()) { faces.emplace_back(face); }

                std::sort(faces.begin(), faces.end(), std::move(func));
                return faces.front();
            }

            orientationAxisSelector face_selectors = {
                {adsk::core::YUpModelingOrientation, yup_faces},
                {adsk::core::ZUpModelingOrientation, zup_faces}
            };

            adsk::core::Ptr<adsk::core::Application> &m_app;
            entt::registry                           &m_registry;
            entt::registry m_renders;

            auto renderSinglePanel(
                const std::string &names,
                const fusion::PanelProfileSketch &sketch,
                const entities::PanelExtrusion &extrusion,
                adsk::core::DefaultModelingOrientations orientation,
                JointRenderData &joints
            ) -> adsk::core::Ptr<adsk::fusion::ExtrudeFeature>;

            static void renderPanelCopies(
                const adsk::core::Ptr<adsk::fusion::ExtrudeFeature> &feature,
                const std::vector<entities::PanelExtrusion> &panels,
                const entities::PanelExtrusion &extrusion
            );

            void renderJointSketches(
                std::vector<std::vector<CutProfile>>& cuts,
                std::string panel_name,
                entities::ExtrusionDistance panel_thickness,
                adsk::core::DefaultModelingOrientations orientation,
                const adsk::core::Ptr<adsk::fusion::ExtrudeFeature> &extrusion,
                JointRenderData &joints
            );

        public:
            ParametricRenderer(adsk::core::Ptr<adsk::core::Application> &app, entt::registry &registry) : m_app{app}, m_registry{registry} {};

            void execute(
                adsk::core::DefaultModelingOrientations orientation,
                const adsk::core::Ptr<adsk::fusion::Component> &component
            ) override;

            void renderJointSketch(
                std::vector<std::vector<CutProfile>> &cuts, std::string panel_name, const entities::ExtrusionDistance &panel_thickness,
                const adsk::core::DefaultModelingOrientations &model_orientation, const adsk::core::Ptr<adsk::fusion::ExtrudeFeature> &extrusion,
                std::string sketch_prefix, const entities::AxisFlag &joint_orientation, const JointRenderProfileGroup &joint_groups
            ) ;
    };
}

#endif /* silvanuspro_render_hpp */
