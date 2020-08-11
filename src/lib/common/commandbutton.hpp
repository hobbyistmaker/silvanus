//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/21/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//

#ifndef SILVANUSPRO_COMMANDBUTTON_HPP
#define SILVANUSPRO_COMMANDBUTTON_HPP

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include <utility>

#include "fusion360command.hpp"
#include "eventhandlers.hpp"

namespace silvanus::common {

        class CommandButton {

            adsk::core::Ptr<adsk::core::Application> m_app;
            std::shared_ptr<Fusion360Command> m_command;
            std::unique_ptr<adsk::core::CommandCreatedEventHandler> m_handler;
            
        public:
            CommandButton(const adsk::core::Ptr<adsk::core::Application>& app, std::shared_ptr<Fusion360Command> command) :
                m_app{app}, m_command{std::move(command)}, m_handler{std::make_unique<CreatedEventHandler>(std::weak_ptr<Fusion360Command>(m_command))} {
                    start();
            };
            adsk::core::Ptr<adsk::core::CommandDefinition> addButtonDefinition();
            void addCommandButton() {
                addControlToPanel();
                registerHandler();
            };
            void addControlToPanel();
            adsk::core::Ptr<adsk::core::CommandControl> commandControl();
            adsk::core::Ptr<adsk::core::CommandDefinition> commandDefinition();
            void delCommandButton();
            adsk::core::Ptr<adsk::core::CommandDefinition> findOrCreateCommandDefinition();
            void registerHandler();
            adsk::core::Ptr<adsk::core::ToolbarPanel> scriptsPanel();
            void start() {
                delCommandButton();
                addCommandButton();
            };
            void stop() {
                delCommandButton();
            };
        };
    
}

#endif /* silvanuspro_commandbutton_hpp */
