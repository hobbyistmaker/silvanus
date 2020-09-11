//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/25/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_DIALOGINPUTS_HPP
#define SILVANUSPRO_DIALOGINPUTS_HPP

#include "entities/AxisFlag.hpp"
#include "entities/Panel.hpp"
#include "entities/Position.hpp"

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <entt/entt.hpp>
#include <map>
#include <set>

namespace silvanus::generatebox::entities
{

    enum class DialogInputs {
        TopThickness, BottomThickness, LeftThickness, RightThickness, FrontThickness, BackThickness,
        TopEnable, BottomEnable, LeftEnable, RightEnable, FrontEnable, BackEnable,
        TopOverride, BottomOverride, LeftOverride, RightOverride, FrontOverride, BackOverride,
        Length, Width, Height, Kerf, FingerWidth, Thickness, FullPreview, FastPreview, ModelSelection,
        FullPreviewLabel, FastPreviewLabel, DividerOrientations, LengthDividerCount, WidthDividerCount,
        HeightDividerCount, FingerMode, DividerLapInput, LengthDividerFBJointInput, LengthDividerTBJointInput,
        WidthDividerLRJointInput, WidthDividerTBJointInput, HeightDividerFBJointInput, HeightDividerLRJointInput
    };

    struct DialogLengthInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct DialogWidthInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct DialogHeightInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct DialogFingerWidthInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct DialogKerfInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct DialogModelType {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogFingerMode {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogFastPreviewMode {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> control;
    };

    struct DialogPanelOverride {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> override;
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> thickness;
    };

    struct DialogFastPreviewLabel {
        adsk::core::Ptr<adsk::core::TextBoxCommandInput> control;
    };

    struct DialogFullPreviewMode {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> control;
    };

    struct DialogFullPreviewLabel {
        adsk::core::Ptr<adsk::core::TextBoxCommandInput> control;
    };

    struct DialogLengthDividerGroupInput {
        adsk::core::Ptr<adsk::core::GroupCommandInput> control;
    };

    struct DialogWidthDividerGroupInput {
        adsk::core::Ptr<adsk::core::GroupCommandInput> control;
    };

    struct DialogHeightDividerGroupInput {
        adsk::core::Ptr<adsk::core::GroupCommandInput> control;
    };

    struct DialogStandardJointGroupInput {
        adsk::core::Ptr<adsk::core::GroupCommandInput> control;
    };

    struct DialogDividerOrientationsInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogDividerJointInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogLengthDividerFrontBackJointInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogLengthDividerTopBottomJointInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogWidthDividerLeftRightJointInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogWidthDividerTopBottomJointInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogHeightDividerFrontBackJointInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogHeightDividerLeftRightJointInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogInsetPanelsInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogLengthDividerCountInput {
        adsk::core::Ptr<adsk::core::IntegerSpinnerCommandInput> control;
    };

    struct DialogWidthDividerCountInput {
        adsk::core::Ptr<adsk::core::IntegerSpinnerCommandInput> control;
    };

    struct DialogHeightDividerCountInput {
        adsk::core::Ptr<adsk::core::IntegerSpinnerCommandInput> control;
    };

    struct DialogModelingUnits {
        bool value;
    };

    struct DialogModelingOrientation {
        adsk::core::DefaultModelingOrientations value;
    };

    struct DialogCommandInputs {
        adsk::core::Ptr<adsk::core::CommandInputs> controls;
    };

    struct DialogApplication {
        adsk::core::Ptr<adsk::core::Application> value;
    };

    struct DialogRootComponent {
        adsk::core::Ptr<adsk::fusion::Component> value;
    };

    struct DialogCreationMode {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogJointDirectionInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
        bool reverse = false;
    };

    struct DialogJointDirectionInputs {
        DialogJointDirectionInput first;
        DialogJointDirectionInput second;
    };

    struct PanelLabelInput {
        adsk::core::Ptr<adsk::core::TextBoxCommandInput> control;
    };

    struct PanelEnableInput {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> control;
    };

    struct PanelOverrideInput {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> control;
    };

    struct DialogJointPatternInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogThicknessInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct DialogTopThickness : public DialogThicknessInput {};
    struct DialogBottomThickness : public DialogThicknessInput {};
    struct DialogLeftThickness : public DialogThicknessInput {};
    struct DialogRightThickness : public DialogThicknessInput {};
    struct DialogFrontThickness : public DialogThicknessInput {};
    struct DialogBackThickness : public DialogThicknessInput {};

    struct DialogErrorMessage {
        std::string value;
    };

    struct DialogPanelId {
        entt::entity id = entt::null;
    };

    struct DialogPanels {
        DialogPanelId first;
        DialogPanelId second;
    };

    struct DialogTopPanel : public DialogPanelId {};
    struct DialogBottomPanel : public DialogPanelId {};
    struct DialogLeftPanel : public DialogPanelId {};
    struct DialogRightPanel : public DialogPanelId {};
    struct DialogFrontPanel : public DialogPanelId {};
    struct DialogBackPanel : public DialogPanelId {};

    struct PanelPlane {
        double min_x = 0.0;
        double min_y = 0.0;
        double max_x = 0.0;
        double max_y = 0.0;
    };

    struct PanelPlaneParams {
        std::string min_x;
        std::string min_y;
        std::string max_x;
        std::string max_y;
    };

    struct PanelPlanes {
        PanelPlane length{};
        PanelPlane width{};
        PanelPlane height{};
    };

    struct PanelPlanesParams {
        PanelPlaneParams length;
        PanelPlaneParams width;
        PanelPlaneParams height;
    };

    struct JointPanelPlanes {
        entt::entity entity = entt::null;
        Panel panel;
        PanelPlanes planes;
    };

    struct JointPanelPlanesParams {
        entt::entity entity = entt::null;
        Panel panel;
        PanelPlanesParams planes;
    };

    enum class DialogJointPatternType {
            Normal, Inverted
    };

    struct DialogEntityName {
        entt::entity entity = entt::null;
        std::string name;
    };

    struct DialogFirstPlanes {
        PanelPlanes planes;
    };

    struct DialogFirstPlanesParams {
        PanelPlanesParams planes;
    };

    struct DialogSecondPlanes {
        PanelPlanes planes;
    };

    struct DialogSecondPlanesParams {
        PanelPlanesParams planes;
    };

    struct JointPanels {
        JointPanelPlanes first;
        JointPanelPlanes second;
    };

    struct JointPanelsParams {
        JointPanelPlanesParams first;
        JointPanelPlanesParams second;
    };

    struct DialogJointPattern {
        DialogJointPatternType protrusion = DialogJointPatternType::Normal;
    };

    struct DialogPanelJointData {
        double             panel_offset = 0;
        double             joint_offset = 0;
        double             distance     = 0;
    };

    struct DialogPanelJointDataParams {
        std::string panel_offset;
        std::string joint_offset;
        std::string distance;
    };

    struct DialogPanelCollisionData {
        DialogPanelJointData first;
        DialogPanelJointData second;
    };

    struct DialogPanelCollisionDataParams {
        DialogPanelJointDataParams first;
        DialogPanelJointDataParams second;
    };

    struct DialogPanelCollisionPair {
        bool               collision_detected = false;
        bool               first_is_primary   = false;
        Position position = Position::Outside;
        DialogJointPattern pattern;
        DialogPanelJointData data;
    };

    struct DialogPanelCollisionPairParams {
        DialogPanelJointDataParams data;
    };

    struct DialogPanelCollisionPairPlanes {
        PanelPlanes first;
        PanelPlanes second;
    };

    struct DialogJointPanelOffsetInput {
        adsk::core::Ptr<adsk::core::TextBoxCommandInput> control;
    };
    struct DialogJointJointOffsetInput {
        adsk::core::Ptr<adsk::core::TextBoxCommandInput> control;
    };
    struct DialogJointDistanceOffsetInput {
        adsk::core::Ptr<adsk::core::TextBoxCommandInput> control;
    };
    struct DialogJointIndex {
        std::map<entt::entity, std::set<entt::entity>> first_panels;
        std::map<entt::entity, std::set<entt::entity>> second_panels;
    };

    struct PanelEnabled {
        bool is_true;
    };

}

#endif /* silvanuspro_inputs_hpp */
