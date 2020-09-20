//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/27/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#include "ParametricRenderer.hpp"

#include "Core/Geometry/Point3D.h"

#include "fusion/FingerCutsPattern.hpp"
#include "fusion/PanelFingerSketch.hpp"
#include "fusion/PanelFeature.hpp"
#include "entities/EntitiesAll.hpp"

#include <map>
#include <set>
#include <string>

#include "plog/Log.h"

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus::generatebox::entities;
using namespace silvanus::generatebox::fusion;
using namespace silvanus::generatebox::render;

using std::map;
using std::unordered_map;

using jointProfileSet = std::set<size_t>;
using panelJointGroup = std::map<jointProfileSet, PanelRenderData>;
using positionPanelGroup = std::map<Position, panelJointGroup>;
using profilePositionGroup = std::map<PanelProfile, positionPanelGroup, ComparePanelProfile>;
using axisProfileGroup = std::map<AxisFlag, profilePositionGroup>;

auto ParametricRenderer::updateFormula(
    const Ptr<Parameter>& parameter, std::string expression
    ) -> void {
    expression.shrink_to_fit();
    auto original = expression;

    if (expression.length() == 0) return;

    auto& existing_params = m_renders.ctx<ExpressionParameterMap>().parameters;

    PLOG_DEBUG << "Updating parameter " << parameter->name() << " for " << expression;

    auto existing_expression = existing_params[expression];
    if (existing_expression.length() > 0) {
        parameter->expression(existing_expression);
    } else {
        parameter->expression(expression);
        PLOG_DEBUG << "Updated parameter " << parameter->name() << " for " << expression;
    }
}

auto ParametricRenderer::updateFormula(
    const Ptr<Parameter>& parameter, const std::string& expression, const std::string& negative
) -> void {
    if (parameter->value() > 0) {
        updateFormula(parameter, expression);
    } else if (parameter->value() < 0) {
        updateFormula(parameter, negative);
    }
}

auto ParametricRenderer::update_parameter(
    Ptr<Parameter> &parameter, FloatParameter& input
) -> void {
    PLOG_DEBUG << "Updating parameter for " << input.name;
    parameter->expression(input.expression);
}

auto ParametricRenderer::create_parameter(
    Ptr<UserParameters> &parameters, FloatParameter& input
) -> void {
    PLOG_DEBUG << "Creating new parameter for " << input.name;
    auto value = ValueInput::createByString(input.expression);
    parameters->add(input.name, value, input.unit_type, "");
}

auto ParametricRenderer::find_or_create_parameter(Ptr<ParameterList>& all_parameters, Ptr<UserParameters>& user_parameters, FloatParameter& input) -> void {
    auto existing = all_parameters->itemByName(input.name);

    if (existing) {
        update_parameter(existing, input);
        return;
    } else {
        create_parameter(user_parameters, input);
    }
}

auto ParametricRenderer::initializeParameters() -> void{
    auto app = adsk::core::Application::get();
    auto all_parameters = Ptr<Design>{app->activeProduct()}->allParameters();
    auto user_parameters = Ptr<Design>{app->activeProduct()}->userParameters();

    auto param_init_view = m_registry.view<FloatParameter>();
    for (auto &&[entity, parameter]: param_init_view.proxy()) {
        find_or_create_parameter(all_parameters, user_parameters, parameter);
    }
}

auto ParametricRenderer::initializePanelGroups() -> void {
    auto panel_groups = axisProfileGroup{};

    auto view = m_registry.view<Enabled, JointEnabled, Panel, PanelGroup, PanelExtrusion, JointGroup, JointName, JointExtrusion>();
    for (auto &&[entity, enabled, joint_enabled, panel, panel_group, panel_extrusion, joint_group, joint_name, joint_extrusion]: view.proxy()) {
        PLOG_DEBUG << panel.name << " enabled == " << (int) enabled.value;
        PLOG_DEBUG << panel.name << " joint to " << joint_name.value << " enabled == " << (int) joint_enabled.value;

        if (!enabled.value || !joint_enabled.value) continue;

        PLOG_DEBUG << "Adding Panel " << panel.name << " with joint to " << joint_name.value << " for direct render";
        PLOG_DEBUG << "Joint direction is " << (int)joint_group.profile.joint_direction;
        PLOG_DEBUG << "Joint thickness is " << joint_group.joint_thickness.value;
        PLOG_DEBUG << "Joint pattern type is " << (int)joint_group.profile.joint_type;

        auto &group = panel_groups[panel_group.orientation][panel_group.profile][panel_group.position][joint_group.tag.value];

        if (group.parent == entt::null) { group.parent = entity; };

        group.names.insert(panel_extrusion.name);
        group.panels[panel_group.distance].insert(panel_extrusion);

        if ((joint_group.profile.finger_type == FingerPatternType::None) || (joint_group.profile.joint_type == JointPatternType::None)) {
            continue;
        }

        auto& joined_panel_group = group.joints[joint_group.profile.joint_type][joint_group.profile.joint_direction][joint_group.profile.joint_orientation][joint_group.profile];
        joined_panel_group.names.insert(joint_name.value);
        joined_panel_group.extrusions.insert(joint_extrusion);
    }

    if (panel_groups.empty()) {
        return;
    }

    m_renders.set<axisProfileGroup>(panel_groups);
}

auto ParametricRenderer::renderPanelGroups(DefaultModelingOrientations model_orientation, const Ptr<Component>& component) -> void {
    auto panel_groups = m_renders.ctx<axisProfileGroup>();

    auto yup_planes   = axis_plane_map{
        {AxisFlag::Height, component->xZConstructionPlane()},
        {AxisFlag::Length, component->yZConstructionPlane()},
        {AxisFlag::Width,  component->xYConstructionPlane()}
    };
    auto zup_planes   = axis_plane_map{
        {AxisFlag::Height, component->xYConstructionPlane()},
        {AxisFlag::Length, component->yZConstructionPlane()},
        {AxisFlag::Width,  component->xZConstructionPlane()}
    };
    auto orientations = orientation_plane_map{
        {YUpModelingOrientation, yup_planes},
        {ZUpModelingOrientation, zup_planes}
    };

    for (auto& [axis, axis_data] : panel_groups) {
        for (auto& [profile, profile_data] : axis_data) {
            for (auto& [position, position_data]: profile_data) {
                for (auto& [joint_profile, joint_group]: position_data) {
                    auto const &plane     = orientations[model_orientation][axis];
                    auto const &transform = sketch_transforms[model_orientation][axis];

                    auto timeline  = Ptr<Design>{m_app->activeProduct()}->timeline();
                    auto start_pos = timeline->markerPosition();

                    auto const names  = concat_names(std::vector<std::string>(joint_group.names.begin(), joint_group.names.end()));
                    auto sketch = PanelProfileSketch(names + " Profile Sketch", plane, transform, profile);

                    if (model_orientation == ZUpModelingOrientation && axis == AxisFlag::Length) { // TODO: This shouldn't be needed
                        updateFormula(sketch.lengthDimension()->parameter(), profile.width.expression);
                        updateFormula(sketch.widthDimension()->parameter(), profile.length.expression);
                    } else {
                        updateFormula(sketch.lengthDimension()->parameter(), profile.length.expression);
                        updateFormula(sketch.widthDimension()->parameter(), profile.width.expression);
                    }

                    for (auto const&[distance, extrusions]: joint_group.panels) {

                        if (extrusions.empty()) { continue; }

                        auto const panels = std::vector<PanelExtrusion>{extrusions.begin(), extrusions.end()};

                        auto const feature = renderSinglePanel(names, sketch, panels[0], model_orientation, joint_group.joints);

                        if (panels.size() == 1) { continue; }

                        renderPanelCopies(feature, panels);
                    }

                    auto const end_pos = timeline->markerPosition() - 1;
                    if ((end_pos - start_pos) <= 0) { continue; }

                    auto const timeline_group = timeline->timelineGroups()->add(start_pos, end_pos);
                    timeline_group->name(names + " Panel Group");
                }
            }
        }
    }
}

void ParametricRenderer::execute(DefaultModelingOrientations model_orientation, const Ptr<Component>& component) {

    m_renders.set<ExpressionParameterMap>();

    initializeParameters();
    initializePanelGroups();
    renderPanelGroups(model_orientation, component);

    m_renders.unset<ExpressionParameterMap>();
    m_renders.clear();
}

auto ParametricRenderer::renderSinglePanel(
    const std::string& names,
    const PanelProfileSketch& sketch,
    const PanelExtrusion& data,
    const DefaultModelingOrientations& model_orientation,
    jointPatternTypeMap& joints
) -> Ptr<ExtrudeFeature> {
    PLOG_DEBUG << "Extruding " << data.name << " with distance " << data.distance.expression << " and offset " << data.offset.expression;
    auto const extrusion = sketch.extrudeProfile(data.distance, data.offset);

    auto distance_param = Ptr<DistanceExtentDefinition>{extrusion->extentOne()}->distance();
    updateFormula(distance_param, data.distance.expression); // Reassign since Fusion appears to throw away the string expression

    extrusion->name(data.name + " Panel Extrusion");
    auto const body = extrusion->bodies()->item(0);
    body->name(data.name + " Panel Body");

    auto cuts = renderJointSketches(names, data, model_orientation, extrusion, joints);

    for (auto const& cut_pairs: cuts) {
        for (auto const& cut: cut_pairs) {
            auto &cut_sketch = cut.sketch;
            auto const &cut_names  = cut.group.names;

            std::vector<Ptr<ExtrudeFeature>> features;

            for (auto const& cut_extrusion: cut.group.extrusions) {
                auto const& cut_feature = cut_sketch.cutJoint(cut_extrusion.offset, cut_extrusion.distance, body);
                auto const& offset_dimension = Ptr<Parameter>{Ptr<FromEntityStartDefinition>{cut_feature->startExtent()}->offset()};
                auto const& distance_dimension = Ptr<DistanceExtentDefinition>{cut_feature->extentOne()}->distance();
                PLOG_DEBUG << "Updating offset dimension: " << offset_dimension->name();
                PLOG_DEBUG << (int)cut_extrusion.joint_id << "Finger cut offset is " << std::to_string(offset_dimension->value()) << " from " << offset_dimension->expression();
                PLOG_DEBUG << (int)cut_extrusion.joint_id << "Finger cut offset expression is " << cut_extrusion.offset.expression;
                PLOG_DEBUG << "Updating distance dimension: " << distance_dimension->name();
                PLOG_DEBUG << (int)cut_extrusion.joint_id << "Finger cut distance expression is " << cut_extrusion.distance.expression;

                auto offset_expression = cut_extrusion.offset.expression;
                offset_expression.shrink_to_fit();
                auto negative_offset = offset_expression.length() > 0 ? "-(" + offset_expression + ")" : "";
                updateFormula(offset_dimension, offset_expression, negative_offset);

                auto distance_expression = cut_extrusion.distance.expression;
                distance_expression.shrink_to_fit();
                auto negative_distance = distance_expression.length() > 0 ? "-(" + distance_expression + ")" : "";
                updateFormula(distance_dimension, distance_expression, negative_distance);

                auto group = cut.group.names;
                auto feature_prefix = names + " " + concat_names({group.begin(), group.end()});
                if (cut.corner) {
                    cut_feature->name(feature_prefix + " Corner Extrusion");
                } else {
                    cut_feature->name(feature_prefix + " Finger Extrusion");
                }
                features.emplace_back(cut_feature);
            }

            if ((cut.profile.finger_count <= 1) && (!cut.corner)) {
                continue;
            }

            auto const &product        = m_app->activeProduct();
            auto const &design         = adsk::core::Ptr<Design>{product};
            auto const &root_component = design->rootComponent();

            auto replicator = FingerCutsPattern(m_app, root_component);

            auto const &copy_feature = replicator.copy(model_orientation, features, cut.profile, cut.corner);
            if (copy_feature) {
                auto feature_prefix = names + " " + concat_names({cut_names.begin(), cut_names.end()});
                if (cut.corner) {
                    auto const& distance = copy_feature->distanceOne();

                    auto distance_expression = cut.profile.parameters.corner_distance;
                    updateFormula(distance, distance_expression);
                    copy_feature->name(feature_prefix.append(" Corner Pattern"));
                } else {
                    auto const& distance = copy_feature->distanceOne();
                    auto const& quantity = copy_feature->quantityOne();

                    auto distance_expression = cut.profile.parameters.pattern_distance;
                    updateFormula(distance, distance_expression);

                    auto quantity_expression = cut.profile.parameters.finger_count;
                    updateFormula(quantity, quantity_expression);
                    copy_feature->name(feature_prefix.append(" Finger Pattern"));
                }
            }
        }
    }

    return extrusion;
}

void ParametricRenderer::renderPanelCopies(
    const Ptr<ExtrudeFeature>& feature,
    const std::vector<PanelExtrusion>& panels
) {
    auto copies = std::vector<PanelExtrusion>{panels.begin() + 1, panels.end()};

    auto      parent = PanelFeature(feature);
    for (auto &panel : copies) {
        auto extrusion = parent.extrudeCopy(panel.distance, panel.offset, panels[0].offset);

        extrusion->name(panel.name + " Panel Extrusion");
        auto body = extrusion->bodies()->item(0);
        body->name(panel.name + " Panel Body");
    }
}

auto ParametricRenderer::renderJointSketches(
    const std::string& panel_name,
    const PanelExtrusion& panel,
    const DefaultModelingOrientations& model_orientation,
    const adsk::core::Ptr<ExtrudeFeature>& extrusion,
    jointPatternTypeMap& joints
) -> std::vector<std::vector<CutProfile>>{
    std::vector<std::vector<CutProfile>> cuts;

    auto name_selector = std::map<JointPatternType, std::string>{
        {JointPatternType::BoxJoint,    "Box Finger"},
        {JointPatternType::LapJoint,    "Lap Finger"},
        {JointPatternType::Trim,        "Trim Finger"},
        {JointPatternType::Tenon,       "Tenon"},
        {JointPatternType::DoubleTenon, "Double Tenon"},
        {JointPatternType::TripleTenon, "Triple Tenon"},
        {JointPatternType::QuadTenon,   "Quad Tenon"}
    };

    for (auto &[joint_type, joint_name]: name_selector){
        for (auto&[joint_direction, direction_data]: joints[joint_type]) {
            for (auto &[joint_orientation, joint_groups]: direction_data) {
                for (auto const&[joint_profile, joint_profile_data]: joint_groups) {
                    cuts.emplace_back(
                        renderJointSketch(panel_name, panel, model_orientation, extrusion, joint_name, joint_orientation, joint_groups));

                    if (joint_profile.corner_width == 0) continue;

                    cuts.emplace_back(
                        renderCornerJointSketch(panel_name, panel, model_orientation, extrusion, joint_name + " Corner", joint_orientation, joint_groups));

                }
            }
        }
    }

    return cuts;
}

auto ParametricRenderer::renderJointSketch(
    const std::string& panel_name,
    const PanelExtrusion& panel,
    const DefaultModelingOrientations &model_orientation,
    const Ptr<ExtrudeFeature> &extrusion,
    const std::string& sketch_prefix,
    const AxisFlag &joint_orientation,
    const profileRenderGroupMap &joint_groups
) -> std::vector<CutProfile>{
    auto profiles    = std::vector<CutProfile>{};
    auto panel_thickness = panel.distance;

    for (auto const& [profile, joints]: joint_groups) {
        if ((profile.joint_direction == JointDirectionType::Inverted) && (profile.finger_count < 1)) {
            continue;
        }
        auto const suffix         = " " + concat_names(std::vector<std::string>(joints.names.begin(), joints.names.end())) + " " + sketch_prefix + " Sketch";
        auto const profile_name   = panel_name + suffix;
        auto const pattern_offset = profile.pattern_offset;
        auto const finger_width   = profile.finger_width;

        auto sketch = PanelFingerSketch(
            extrusion,
            face_selectors[model_orientation][joint_orientation],
            Point3D::create(pattern_offset, 0, 0),
            Point3D::create(finger_width, panel_thickness.value, 0),
            profile_name
        );
        PLOG_DEBUG << "Finger profile width: " << profile.parameters.finger_width;
        if (profile.parameters.finger_width.length() > 0) sketch.fingerLength()->expression(profile.parameters.finger_width);
        if (profile.parameters.pattern_offset.length() > 0) sketch.originOffset()->expression(profile.parameters.pattern_offset);

        profiles.emplace_back(CutProfile{sketch, joints, profile});
    }

    return profiles;
}

auto ParametricRenderer::renderCornerJointSketch(
    const std::string& panel_name,
    const PanelExtrusion& panel,
    const DefaultModelingOrientations& model_orientation,
    const Ptr<ExtrudeFeature>& extrusion,
    const std::string& sketch_prefix,
    const AxisFlag& joint_orientation,
    const profileRenderGroupMap& joint_groups
) -> std::vector<CutProfile>{
    auto pairs    = std::vector<CutProfile>{};
    auto panel_thickness = panel.distance;

    for (auto const& [profile, joints]: joint_groups) {
        auto const suffix         = " " + concat_names(std::vector<std::string>(joints.names.begin(), joints.names.end())) + " " + sketch_prefix + " Sketch";
        auto const profile_name   = panel_name + suffix;
        auto const pattern_offset = 0;
        auto const finger_width   = profile.corner_width;

        auto sketch = PanelFingerSketch(
            extrusion,
            face_selectors[model_orientation][joint_orientation],
            Point3D::create(pattern_offset, 0, 0),
            Point3D::create(finger_width, panel_thickness.value, 0),
            profile_name
        );
        sketch.fingerLength()->expression(profile.parameters.corner_width);
        pairs.emplace_back(CutProfile{sketch, joints, profile, true});
    }

    return pairs;
}
