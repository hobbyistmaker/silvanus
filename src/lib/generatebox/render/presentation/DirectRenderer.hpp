//
// Created by Hobbyist Maker on 8/5/20.
//

#ifndef SILVANUSPRO_DIRECTRENDERER_HPP
#define SILVANUSPRO_DIRECTRENDERER_HPP

#include "rendersupport.hpp"

#include "Renderer.hpp"
#include "entities/AxisFlag.hpp"
#include "entities/JointExtrusion.hpp"
#include "entities/JointProfile.hpp"
#include "entities/PanelExtrusion.hpp"
#include "entities/PanelProfile.hpp"
#include "entities/Position.hpp"

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <entt/entt.hpp>

#include <map>
#include <set>

namespace silvanus::generatebox::render {

    using adsk::core::Point3D;
    using adsk::core::Vector3D;

    using entities::AxisFlag;
    using entities::ComparePanelProfile;
    using entities::PanelProfile;
    using entities::Position;

    class DirectRenderer : public Renderer {

            using jointTransformVectorList = std::vector<adsk::core::Ptr<Vector3D>>;
            using jointTransformVectorFingerAxisMap = std::map<AxisFlag, jointTransformVectorList>;
            using jointTransformVectorPanelAxisMap = std::map<AxisFlag, jointTransformVectorFingerAxisMap>;
            using jointTransformVectorOrientationMap = std::map<adsk::core::DefaultModelingOrientations, jointTransformVectorPanelAxisMap>;

            jointTransformVectorPanelAxisMap joint_yup_vectors = {
                {AxisFlag::Length, {
                    {AxisFlag::Height, {Vector3D::create(0.0, 0.0, 1.0), Vector3D::create(0.0, 1.0, 0.0)} },
                    {AxisFlag::Width, {Vector3D::create(0.0, 1.0, 0.0), Vector3D::create(0.0, 0.0, 1.0)} }
                }},
                {AxisFlag::Width, {
                    {AxisFlag::Length, {Vector3D::create(0.0, 1.0, 0.0), Vector3D::create(1.0, 0.0, 0.0)} },
                    {AxisFlag::Height, {Vector3D::create(1.0, 0.0, 0.0), Vector3D::create(0.0, 1.0, 0.0)} }
                }},
                {AxisFlag::Height, {
                    {AxisFlag::Width, {Vector3D::create(1.0, 0.0, 0.0), Vector3D::create(0.0, 0.0, 1.0)} },
                    {AxisFlag::Length, {Vector3D::create(0.0, 0.0, 1.0), Vector3D::create(1.0, 0.0, 0.0)} }
                }}
            };
            jointTransformVectorPanelAxisMap joint_zup_vectors = {
                {AxisFlag::Length, {
                    {AxisFlag::Height, {Vector3D::create(0.0, 1.0, 0.0), Vector3D::create(0.0, 0.0, 1.0)} },
                    {AxisFlag::Width, {Vector3D::create(0.0, 0.0, 1.0), Vector3D::create(0.0, 1.0, 0.0)} }
                }},
                {AxisFlag::Width, {
                    {AxisFlag::Length, {Vector3D::create(0.0, 0.0, 1.0), Vector3D::create(1.0, 0.0, 0.0)} },
                    {AxisFlag::Height, {Vector3D::create(1.0, 0.0, 0.0), Vector3D::create(0.0, 0.0, 1.0)} }
                }},
                {AxisFlag::Height, {
                    {AxisFlag::Width, {Vector3D::create(1.0, 0.0, 0.0), Vector3D::create(0.0, 1.0, 0.0)} },
                    {AxisFlag::Length, {Vector3D::create(0.0, 1.0, 0.0), Vector3D::create(1.0, 0.0, 0.0)} }
                }}
            };
            jointTransformVectorOrientationMap joint_orientation_selector = {
                {adsk::core::YUpModelingOrientation, joint_yup_vectors},
                {adsk::core::ZUpModelingOrientation, joint_zup_vectors}
            };

            using jointTransformVectorFunctionList = std::function<adsk::core::Ptr<Vector3D>(double, double, double)>;
            using jointTransformVectorFunctionFingerAxisMap = std::map<AxisFlag, jointTransformVectorFunctionList>;
            using jointTransformVectorFunctionPanelAxisMap = std::map<AxisFlag, jointTransformVectorFunctionFingerAxisMap>;
            using jointTransformVectorFunctionOrientationMap = std::map<adsk::core::DefaultModelingOrientations, jointTransformVectorFunctionPanelAxisMap>;

            jointTransformVectorFunctionPanelAxisMap joint_yup_transforms = {
                { AxisFlag::Length, {
                    {AxisFlag::Height, { [](double l, double w, double h){ return Vector3D::create(w, h, l); } } },
                    {AxisFlag::Width, { [](double l, double w, double h){ return Vector3D::create(h, l, w); } } }
                }},
                { AxisFlag::Width, {
                    {AxisFlag::Length, { [](double l, double w, double h){ return Vector3D::create(w, l, h); } } },
                    {AxisFlag::Height, { [](double l, double w, double h){ return Vector3D::create(l, w, h); } } }
                }},
                { AxisFlag::Height, {
                    {AxisFlag::Width, { [](double l, double w, double h){ return Vector3D::create(l, h, w); } } },
                    {AxisFlag::Length, { [](double l, double w, double h){ return Vector3D::create(h, w, l); } } }
                }}
            };
            jointTransformVectorFunctionPanelAxisMap joint_zup_transforms = {
                { AxisFlag::Length, {
                    {AxisFlag::Height, { [](double l, double w, double h){ return Vector3D::create(w, l, h); } } },
                    {AxisFlag::Width, { [](double l, double w, double h){ return Vector3D::create(h, w, l); } } }
                }},
                { AxisFlag::Width, {
                    {AxisFlag::Length, { [](double l, double w, double h){ return Vector3D::create(w, h, l); } } },
                    {AxisFlag::Height, { [](double l, double w, double h){ return Vector3D::create(l, h, w); } } }
                }},
                { AxisFlag::Height, {
                    {AxisFlag::Width, { [](double l, double w, double h){ return Vector3D::create(l, w, h); } } },
                    {AxisFlag::Length, { [](double l, double w, double h){ return Vector3D::create(h, l, w); } } }
                }}
            };
            jointTransformVectorFunctionOrientationMap joint_transform_selector = {
                {adsk::core::YUpModelingOrientation, joint_yup_transforms},
                {adsk::core::ZUpModelingOrientation, joint_zup_transforms}
            };

            using jointCenterFunction = std::function<adsk::core::Ptr<Point3D>(double, double, double, double)>;
            using jointCenterFingerAxisMap = std::map<AxisFlag, jointCenterFunction>;
            using jointCenterPanelAxisMap = std::map<AxisFlag, jointCenterFingerAxisMap>;
            using jointCenterOrientationMap = std::map<adsk::core::DefaultModelingOrientations, jointCenterPanelAxisMap>;

            jointCenterPanelAxisMap joint_yup_centers =  {
                {AxisFlag::Length, {
                     {AxisFlag::Height, [](double po, double pd, double jo, double jd){ return Point3D::create( po + pd/2, jo + jd/2, 0 ); }},
                     {AxisFlag::Width, [](double po, double pd, double jo, double jd){ return Point3D::create( po + pd/2, 0, jo + jd/2); }}
                 }},
                {AxisFlag::Width, {
                     {AxisFlag::Length, [](double po, double pd, double jo, double jd){ return Point3D::create( jo + jd/2, 0, po + pd/2 ); }},
                     {AxisFlag::Height, [](double po, double pd, double jo, double jd) { return Point3D::create( 0, jo + jd/2, po + pd/2 ); }}
                 }},
                {AxisFlag::Height , {
                     {AxisFlag::Width, [](double po, double pd, double jo, double jd){ return Point3D::create( 0, po + pd/2, jo + jd/2 ); }},
                     {AxisFlag::Length, [](double po, double pd, double jo, double jd){ return Point3D::create( jo + jd/2, po + pd/2, 0 ); }}
                 }}
            };
            jointCenterPanelAxisMap joint_zup_centers = {
                {AxisFlag::Length, {
                     {AxisFlag::Height, [](double po, double pd, double jo, double jd) { return Point3D::create( po + pd/2, 0, jo + jd/2); }},
                     {AxisFlag::Width, [](double po, double pd, double jo, double jd){ return Point3D::create( po + pd/2, jo + jd/2, 0 ); }}
                 }},
                {AxisFlag::Width, {
                     {AxisFlag::Length, [](double po, double pd, double jo, double jd) { return Point3D::create( jo + jd/2, po + pd/2, 0 ); }},
                     {AxisFlag::Height, [](double po, double pd, double jo, double jd) { return Point3D::create( 0, po + pd/2, jo + jd/2 ); }}
                 }},
                {AxisFlag::Height, {
                     {AxisFlag::Width, [](double po, double pd, double jo, double jd){ return Point3D::create( 0, jo + jd/2, po + pd/2 ); }},
                     {AxisFlag::Length, [](double po, double pd, double jo, double jd){ return Point3D::create( jo + jd/2, 0, po + pd/2 ); }}
                 }}
            };
            jointCenterOrientationMap joint_center_selector = {
                {adsk::core::YUpModelingOrientation, joint_yup_centers},
                {adsk::core::ZUpModelingOrientation, joint_zup_centers}
            };

            using transformVectorList = std::vector<adsk::core::Ptr<Vector3D>>;
            using transformVectorAxisMap = std::map<AxisFlag, transformVectorList>;
            using transformVectorOrientationMap = std::map<adsk::core::DefaultModelingOrientations, transformVectorAxisMap>;

            transformVectorAxisMap yup_vectors = {
                {AxisFlag::Height, {Vector3D::create(1.0, 0.0, 0.0), Vector3D::create(0.0, 0.0, 1.0)} },
                {AxisFlag::Length, {Vector3D::create(0.0, 0.0, 1.0), Vector3D::create(0.0, 1.0, 0.0)} },
                {AxisFlag::Width, {Vector3D::create(1.0, 0.0, 0.0), Vector3D::create(0.0, 1.0, 0.0)} }
            };
            transformVectorAxisMap zup_vectors = {
                {AxisFlag::Height, {Vector3D::create(1.0, 0.0, 0.0), Vector3D::create(0.0, 1.0, 0.0)} },
                {AxisFlag::Length, {Vector3D::create(0.0, 1.0, 0.0), Vector3D::create(0.0, 0.0, 1.0)} },
                {AxisFlag::Width, {Vector3D::create(1.0, 0.0, 0.0), Vector3D::create(0.0, 0.0, 1.0)} }
            };
            transformVectorOrientationMap orientation_selector = {
                {adsk::core::YUpModelingOrientation, yup_vectors},
                {adsk::core::ZUpModelingOrientation, zup_vectors}
            };

            using transformVectorFunctionList = std::function<adsk::core::Ptr<Vector3D>(double, double, double)>;
            using transformVectorFunctionAxisMap = std::map<AxisFlag, transformVectorFunctionList>;
            using transformVectorFunctionOrientationMap = std::map<adsk::core::DefaultModelingOrientations, transformVectorFunctionAxisMap>;

            transformVectorFunctionAxisMap yup_transforms = {
                {AxisFlag::Height, { [](double l, double w, double h){ return Vector3D::create(l, h, w); } } },
                {AxisFlag::Length, { [](double l, double w, double h){ return Vector3D::create(h, w, l); } } },
                {AxisFlag::Width, { [](double l, double w, double h){ return Vector3D::create(l, w, h); } } }
            };
            transformVectorFunctionAxisMap zup_transforms = {
                {AxisFlag::Height, { [](double l, double w, double h){ return Vector3D::create(l, w, h); } } },
                {AxisFlag::Length, { [](double l, double w, double h){ return Vector3D::create(h, l, w); } } },
                {AxisFlag::Width, { [](double l, double w, double h){ return Vector3D::create(l, h, w); } } }
            };
            transformVectorFunctionOrientationMap transform_selector = {
                {adsk::core::YUpModelingOrientation, yup_transforms},
                {adsk::core::ZUpModelingOrientation, zup_transforms}
            };

            using copyTransformVectorFunctionList = std::function<adsk::core::Ptr<Vector3D>(double)>;
            using copyTransformVectorFunctionAxisMap = std::map<AxisFlag, copyTransformVectorFunctionList>;
            using copyTransformVectorFunctionOrientationMap = std::map<adsk::core::DefaultModelingOrientations, copyTransformVectorFunctionAxisMap>;

            copyTransformVectorFunctionAxisMap copy_yup_transforms = {
                {AxisFlag::Height, { [](double o){ return Vector3D::create(0, o, 0); } } },
                {AxisFlag::Length, { [](double o){ return Vector3D::create(o, 0, 0); } } },
                {AxisFlag::Width, { [](double o){ return Vector3D::create(0, 0, o); } } }
            };
            copyTransformVectorFunctionAxisMap copy_zup_transforms = {
                {AxisFlag::Height, { [](double o){ return Vector3D::create(0, 0, o); } } },
                {AxisFlag::Length, { [](double o){ return Vector3D::create(o, 0, 0); } } },
                {AxisFlag::Width, { [](double o){ return Vector3D::create(0, o, 0); } } }
            };
            copyTransformVectorFunctionOrientationMap copy_transform_selector = {
                {adsk::core::YUpModelingOrientation, copy_yup_transforms},
                {adsk::core::ZUpModelingOrientation, copy_zup_transforms}
            };

            using centerFunction = std::function<adsk::core::Ptr<Point3D>(double, double, double, double)>;
            using centerAxisMap = std::map<AxisFlag, centerFunction>;
            using centerOrientationMap = std::map<adsk::core::DefaultModelingOrientations, centerAxisMap>;

            centerAxisMap yup_centers =  {
                {AxisFlag::Height, [](double l, double w, double h, double o){ return Point3D::create( l, w+o, h ); } },
                {AxisFlag::Length, [](double l, double w, double h, double o){ return Point3D::create( l+o, w, h ); } },
                {AxisFlag::Width, [](double l, double w, double h, double o){ return Point3D::create( l, w, h+o ); } }
            };
            centerAxisMap zup_centers = {
                {AxisFlag::Height, [](double l, double w, double h, double o){ return Point3D::create( l, h, w+o ); } },
                {AxisFlag::Length, [](double l, double w, double h, double o){ return Point3D::create( l+o, h, w ); } },
                {AxisFlag::Width, [](double l, double w, double h, double o){ return Point3D::create( l, h+o, w ); } }
            };
            centerOrientationMap center_selector = {
                {adsk::core::YUpModelingOrientation, yup_centers},
                {adsk::core::ZUpModelingOrientation, zup_centers}
            };

            adsk::core::Ptr<adsk::core::Application>& m_app;
            entt::registry& m_registry;

            adsk::core::Ptr<adsk::fusion::TemporaryBRepManager> m_temp_mgr;
            adsk::core::Ptr<adsk::fusion::BRepBodies> m_bodies;

        public:

            DirectRenderer(adsk::core::Ptr<adsk::core::Application>& app, entt::registry& registry)
                : m_app{app}, m_registry{registry}, m_temp_mgr{adsk::fusion::TemporaryBRepManager::get()} {
                auto const& product = m_app->activeProduct();
                auto const& design = adsk::core::Ptr<adsk::fusion::Design>{product};

                auto const& root = design->rootComponent();
                m_bodies = root->bRepBodies();
            };

            void execute(
                adsk::core::DefaultModelingOrientations orientation,
                const adsk::core::Ptr<adsk::fusion::Component>& component
            ) override;

            void renderNormalJoints(
                const adsk::core::DefaultModelingOrientations &model_orientation,
                AxisFlag axis,
                const PanelRenderData& group_data,
                const entities::PanelExtrusion &panel,
                const adsk::core::Ptr<adsk::fusion::BRepBody> &box,
                const renderJointTypeMap& joints_group
            );

            void renderNormalJoint(
                const adsk::core::DefaultModelingOrientations &model_orientation, const AxisFlag &axis, const entities::PanelExtrusion &panel,
                const adsk::core::Ptr<adsk::fusion::BRepBody> &box, const AxisFlag &joint_orientation, const entities::JointProfile &joint_profile,
                const JointRenderGroup &joint_profile_data
            );

            void renderCornerJoint(
                const adsk::core::DefaultModelingOrientations &model_orientation, const AxisFlag &axis, const entities::PanelExtrusion &panel,
                const adsk::core::Ptr<adsk::fusion::BRepBody> &box, const AxisFlag &joint_orientation, const entities::JointProfile &joint_profile,
                const JointRenderGroup &joint_profile_data
            );

            void processPanelGroups(
                const adsk::core::DefaultModelingOrientations &model_orientation,
                const std::map<AxisFlag, std::map<PanelProfile, std::map<Position, std::map<std::set<size_t>, PanelRenderData>>, ComparePanelProfile>> &panel_groups
            );
    };

}

#endif //SILVANUSPRO_DIRECTRENDERER_HPP
