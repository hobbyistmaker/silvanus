//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/22/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//

#ifndef SILVANUSPRO_GENERATEBOXCOMMAND_HPP
#define SILVANUSPRO_GENERATEBOXCOMMAND_HPP

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include "common/common.h"
#include "generatebox/dialog.hpp"
#include "generatebox/systems/SilvanusCore.hpp"

namespace silvanus {

    class GenerateBoxCommand : public common::Fusion360Command {
     
        const std::string description = "Create a parametric box with finger joints.";
        const std::string id = "SilvanusProGenerateBoxCommand";
        const std::string dialog = "Create Finger Jointed Box (0.5.6 alpha)";

        adsk::core::Ptr<adsk::core::UserInterface> m_ui;

        entt::registry m_registry;
        generatebox::CreateDialog command_dialog = generatebox::CreateDialog(m_registry);
        generatebox::systems::SilvanusCore m_core = generatebox::systems::SilvanusCore(m_app, m_registry);

    public:
        explicit GenerateBoxCommand(
             const adsk::core::Ptr<adsk::core::Application>& app
        );
        
        void onCreate(const adsk::core::Ptr<adsk::core::CommandCreatedEventArgs>& args) override;
        bool onChange(const adsk::core::Ptr<adsk::core::InputChangedEventArgs>& args) override;
        void onDestroy(const adsk::core::Ptr<adsk::core::CommandEventArgs>& args) override;
        void onExecute(const adsk::core::Ptr<adsk::core::CommandEventArgs>& args) override;
        void onPreview(const adsk::core::Ptr<adsk::core::CommandEventArgs>& args) override;
        bool onValidate(const adsk::core::Ptr<adsk::core::ValidateInputsEventArgs>& args) override;

        void postValidateValid(const adsk::core::Ptr<adsk::core::CommandEventArgs>& args) override;
        
        std::string getDescription() override { return description; };
        std::string getId() override { return id; };
        std::string getDialogName() override { return dialog; };
    };
}

#endif /* silvanuspro_generateboxcommand_hpp */
