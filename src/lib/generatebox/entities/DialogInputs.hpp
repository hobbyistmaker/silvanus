//
// SilvanusPro
//
// Created by Hobbyist Maker on 7/25/20.
// Copyright (c) 2020 Hobbyist Maker. All rights reserved.
//

#ifndef SILVANUSPRO_DIALOGINPUTS_HPP
#define SILVANUSPRO_DIALOGINPUTS_HPP

#include "Core/CoreAll.h"
#include "Fusion/FusionAll.h"
#include "entt/entt.hpp"

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

    struct DialogBoxLengthInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct DialogBoxWidthInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct DialogBoxHeightInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct DialogBoxThicknessInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct DialogBoxFingerWidthInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct DialogBoxKerfInput {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct DialogBoxModelType {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogBoxFingerMode {
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

    struct DialogBoxDividerJointInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogBoxLengthDividerJointInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogBoxWidthDividerJointInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogBoxInsetPanelsInput {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogBoxLengthDividerCountInput {
        adsk::core::Ptr<adsk::core::IntegerSpinnerCommandInput> control;
    };

    struct DialogBoxWidthDividerCountInput {
        adsk::core::Ptr<adsk::core::IntegerSpinnerCommandInput> control;
    };

    struct DialogBoxHeightDividerCountInput {
        adsk::core::Ptr<adsk::core::IntegerSpinnerCommandInput> control;
    };

    struct DialogBoxModelingUnits {
        bool value;
    };

    struct DialogBoxModelingOrientation {
        adsk::core::DefaultModelingOrientations value;
    };

    struct DialogBoxCommandInputs {
        adsk::core::Ptr<adsk::core::CommandInputs> controls;
    };

    struct DialogBoxApplication {
        adsk::core::Ptr<adsk::core::Application> value;
    };

    struct DialogBoxRootComponent {
        adsk::core::Ptr<adsk::fusion::Component> value;
    };

    struct DialogBoxCreationMode {
        adsk::core::Ptr<adsk::core::DropDownCommandInput> control;
    };

    struct DialogBoxPanelInputs {
        adsk::core::Ptr<adsk::core::TextBoxCommandInput> label;
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> enabled;
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> override;
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> thickness;
    };

    struct DialogBoxTopInputs : DialogBoxPanelInputs {};
    struct DialogBoxBottomInputs : DialogBoxPanelInputs {};
    struct DialogBoxLeftInputs : DialogBoxPanelInputs {};
    struct DialogBoxRightInputs : DialogBoxPanelInputs {};
    struct DialogBoxFrontInputs : DialogBoxPanelInputs {};
    struct DialogBoxBackInputs : DialogBoxPanelInputs {};

    struct DialogBoxPanelLabel {
        adsk::core::Ptr<adsk::core::TextBoxCommandInput> control;
    };

    struct DialogBoxPanelEnable {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> control;
    };

    struct DialogBoxPanelOverride {
        adsk::core::Ptr<adsk::core::BoolValueCommandInput> control;
    };

    struct DialogBoxPanelThickness {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct DialogBoxTopThickness : public DialogBoxPanelThickness {};
    struct DialogBoxBottomThickness : public DialogBoxPanelThickness {};
    struct DialogBoxLeftThickness : public DialogBoxPanelThickness {};
    struct DialogBoxRightThickness : public DialogBoxPanelThickness {};
    struct DialogBoxFrontThickness : public DialogBoxPanelThickness {};
    struct DialogBoxBackThickness : public DialogBoxPanelThickness {};

    struct DialogBoxPanelOverrideThickness {
        adsk::core::Ptr<adsk::core::FloatSpinnerCommandInput> control;
    };

    struct DialogBoxErrorMessage {
        std::string value;
    };

    struct DialogBoxPanel {
        entt::entity id;
    };

    struct DialogBoxTopPanel : public DialogBoxPanel {};
    struct DialogBoxBottomPanel : public DialogBoxPanel {};
    struct DialogBoxLeftPanel : public DialogBoxPanel {};
    struct DialogBoxRightPanel : public DialogBoxPanel {};
    struct DialogBoxFrontPanel : public DialogBoxPanel {};
    struct DialogBoxBackPanel : public DialogBoxPanel {};

}

#endif /* silvanuspro_inputs_hpp */
