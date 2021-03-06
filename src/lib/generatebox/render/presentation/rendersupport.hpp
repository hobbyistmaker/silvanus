//
// Created by Hobbyist Maker on 8/5/20.
//

#ifndef SILVANUSPRO_RENDERSUPPORT_HPP
#define SILVANUSPRO_RENDERSUPPORT_HPP

#include "entities/JointDirection.hpp"
#include "entities/JointExtrusion.hpp"
#include "entities/JointProfile.hpp"
#include "entities/PanelExtrusion.hpp"
#include "fusion/PanelFingerSketch.hpp"

#include <map>
#include <set>
#include <unordered_map>

namespace silvanus::generatebox::render {

    struct CompareExtrusion {
        bool operator()(const entities::PanelExtrusion &lhs, const entities::PanelExtrusion &rhs) const {
            return (lhs.distance.value < rhs.distance.value) || (
                       (rhs.distance.value >= lhs.distance.value) && (lhs.offset.value < rhs.offset.value)
                   );
        }

        bool operator()(const entities::JointExtrusion &lhs, const entities::JointExtrusion &rhs) const {
            return (lhs.distance.value < rhs.distance.value) || (
                       (rhs.distance.value >= lhs.distance.value) && (lhs.offset.value < rhs.offset.value)
                   );
        }
    };

    struct JointRenderGroup {
        std::set<std::string>                                names;
        std::set<entities::JointExtrusion, CompareExtrusion> extrusions;
    };

    using profileRenderGroupMap = std::map<entities::JointProfile, JointRenderGroup, entities::CompareJointProfile>;
    using renderJointTypeMap = std::map<entities::AxisFlag, profileRenderGroupMap>;
    using jointDirectionTypeMap = std::map<entities::JointDirectionType, renderJointTypeMap>;
    using jointPatternTypeMap = std::map<entities::JointPatternType, jointDirectionTypeMap>;
    using panelExtrusionSet = std::set<entities::PanelExtrusion, CompareExtrusion>;
    using distanceExtrusionMap = std::map<entities::ExtrusionDistance, panelExtrusionSet, entities::CompareExtrusionDistance>;

    struct PanelRenderData {
        std::set<std::string> names;
        distanceExtrusionMap  panels;
        jointPatternTypeMap   joints;
        entt::entity          parent = entt::null;
    };

    struct CutProfile {
        fusion::PanelFingerSketch sketch;
        JointRenderGroup          group;
        entities::JointProfile    profile;
        bool                      corner = false;
    };

    using axis_plane_map = std::map<entities::AxisFlag, adsk::core::Ptr<adsk::fusion::ConstructionPlane>>;
    using orientation_plane_map = std::unordered_map<adsk::core::DefaultModelingOrientations, axis_plane_map>;
    using axis_transform_map = std::map<entities::AxisFlag, std::function<std::tuple<double, double, double>(double, double, double)>>;
    using orientation_transform_map = std::unordered_map<adsk::core::DefaultModelingOrientations, axis_transform_map>;
    using axisFaceSelector = std::map<entities::AxisFlag, std::function<adsk::core::Ptr<adsk::fusion::BRepFace>(adsk::core::Ptr<adsk::fusion::BRepBody>)>>;
    using orientationAxisSelector = std::map<adsk::core::DefaultModelingOrientations, axisFaceSelector>;

}

#endif //SILVANUSPRO_RENDERSUPPORT_HPP
