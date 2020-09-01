//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/25/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_DIALOGINPUTS_HPP
#define SILVANUSPRO_DIALOGINPUTS_HPP

#include "entities/AxisFlag.hpp"
#include "entities/Position.hpp"

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <entt/entt.hpp>
#include <map>

namespace silvanus::generatebox::entities
{

    enum class DialogInputs {
        TopThickness, BottomThickness, LeftThickness, RightThickness, FrontThickness, BackThickness,
        TopEnable, BottomEnable, LeftEnable, RightEnable, FrontEnable, BackEnable,
        TopOverride, BottomOverride, LeftOverride, RightOverride, FrontOverride, BackOverride,
        Length, Width, Height, Kerf, FingerWidth, Thickness, FullPreview, FastPreview, ModelSelection,
        FullPreviewLabel, FastPreviewLabel, LengthDividerCount, WidthDividerCount, HeightDividerCount,
        FingerMode, DividerLapInput, LengthDividerJointInput, WidthDividerJointInput
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

    struct DialogThicknessInput {
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

    struct DialogFastPreviewLabel {
        adsk::core::Ptr<adsk::core::TextBoxCommandInput> control;
    };

    struct DialogFullPreviewMode {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> control;
    };

    struct DialogFullPreviewLabel {
        adsk::core::Ptr<adsk::core::TextBoxCommandInput> control;
    };

    struct DialogDividerJointInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogLengthDividerJointInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogWidthDividerJointInput {
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

    struct DialogPanelInputs {
        adsk::core::Ptr<adsk::core::TextBoxCommandInput> label;
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> enabled;
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> override;
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> thickness;
    };

    struct DialogTopInputs : DialogPanelInputs {};
    struct DialogBottomInputs : DialogPanelInputs {};
    struct DialogLeftInputs : DialogPanelInputs {};
    struct DialogRightInputs : DialogPanelInputs {};
    struct DialogFrontInputs : DialogPanelInputs {};
    struct DialogBackInputs : DialogPanelInputs {};

    struct DialogPanelLabel {
        adsk::core::Ptr<adsk::core::TextBoxCommandInput> control;
    };

    struct DialogPanelEnable {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> control;
    };

    struct DialogPanelOverride {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> control;
    };

    struct DialogPanelThickness {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct DialogJointPatternInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogTopThickness : public DialogPanelThickness {};
    struct DialogBottomThickness : public DialogPanelThickness {};
    struct DialogLeftThickness : public DialogPanelThickness {};
    struct DialogRightThickness : public DialogPanelThickness {};
    struct DialogFrontThickness : public DialogPanelThickness {};
    struct DialogBackThickness : public DialogPanelThickness {};

    struct DialogPanelOverrideThickness {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct DialogErrorMessage {
        std::string value;
    };

    struct DialogPanel {
        std::string name;
        int priority;
        entities::AxisFlag orientation;
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

    struct DialogPanelPlane {
        double min_x = 0.0;
        double min_y = 0.0;
        double max_x = 0.0;
        double max_y = 0.0;
    };

    struct DialogPanelPlanes {
        DialogPanelPlane length{};
        DialogPanelPlane width{};
        DialogPanelPlane height{};
    };

    struct DialogPanelJoint {
        entt::entity entity = entt::null;
        DialogPanel panel;
        DialogPanelPlanes planes;
    };


    enum class DialogJointPatternType {
            Normal, Inverted
    };

    struct DialogEntityName {
        entt::entity entity = entt::null;
        std::string name;
    };

    struct DialogPanelCollisionPairPlanes {
        DialogPanelPlanes first;
        DialogPanelPlanes second;
    };

    struct DialogFirstPlanes {
        DialogPanelPlanes planes;
    };

    struct DialogSecondPlanes {
        DialogPanelPlanes planes;
    };

    struct DialogJoints {
        DialogPanelJoint first;
        DialogPanelJoint second;
    };

    struct DialogJointPattern {
        DialogJointPatternType protrusion = DialogJointPatternType::Normal;
    };

    struct DialogPanelJointData {
        double             panel_offset = 0;
        double             joint_offset = 0;
        double             distance     = 0;
    };

    struct DialogPanelCollisionData {
        DialogPanelJointData first;
        DialogPanelJointData second;
    };

    struct DialogPanelCollisionPair {
        bool               collision_detected = false;
        bool               first_is_primary   = false;
        Position position = Position::Outside;
        DialogJointPattern pattern;
        DialogPanelJointData data;
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

    struct DialogPanelEnableValue {
        bool value;
    };
}

#endif /* silvanuspro_inputs_hpp */
