//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/21/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//

#ifndef SILVANUSPRO_EVENTHANDLERS_HPP
#define SILVANUSPRO_EVENTHANDLERS_HPP

#include <memory>
#include <utility>

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#include "fusion360command.hpp"

namespace silvanus::common {

    class CommandDestroyHandler : public adsk::core::CommandEventHandler {
    
        std::weak_ptr<Fusion360Command> m_command;
    
    public:
        explicit CommandDestroyHandler(std::weak_ptr<Fusion360Command> command) : m_command{std::move( command )} {};
        void notify(const adsk::core::Ptr<adsk::core::CommandEventArgs>& args) override {
            m_command.lock()->destroy(args);
        };
    };
    
    class CommandExecuteHandler : public adsk::core::CommandEventHandler {

        std::weak_ptr<Fusion360Command> m_command;

    public:
        explicit CommandExecuteHandler(std::weak_ptr<Fusion360Command> command) : m_command{std::move( command )} {};
        void notify(const adsk::core::Ptr<adsk::core::CommandEventArgs>& args) override {
            m_command.lock()->execute(args);
        };
    };
    
    class InputChangedHandler : public adsk::core::InputChangedEventHandler {

        std::weak_ptr<Fusion360Command> m_command;

    public:
        explicit InputChangedHandler(std::weak_ptr<Fusion360Command> command) : m_command{std::move( command )} {};
        void notify(const adsk::core::Ptr<adsk::core::InputChangedEventArgs>& args) override {
            m_command.lock()->change(args);
        };
    };
    
    class ExecutePreviewHandler : public adsk::core::CommandEventHandler {

        std::weak_ptr<Fusion360Command> m_command;

    public:
        explicit ExecutePreviewHandler(std::weak_ptr<Fusion360Command> command) : m_command{std::move( command )} {};
        void notify(const adsk::core::Ptr<adsk::core::CommandEventArgs>& args) override {
            m_command.lock()->preview(args);
        };
    };
    
    class ValidateInputsHandler : public adsk::core::ValidateInputsEventHandler {

            std::weak_ptr<Fusion360Command> m_command;

    public:
        explicit ValidateInputsHandler(std::weak_ptr<Fusion360Command> command) : m_command{std::move( command )} {};
        void notify(const adsk::core::Ptr<adsk::core::ValidateInputsEventArgs>& args) override {
            args->areInputsValid(m_command.lock()->validate(args));
        };
    };
    
    class CreatedEventHandler : public adsk::core::CommandCreatedEventHandler {

        std::weak_ptr<Fusion360Command> m_command;
        CommandDestroyHandler on_destroy_handler;
        CommandExecuteHandler on_execute_handler;
        InputChangedHandler on_change_handler;
        ExecutePreviewHandler on_preview_handler;
        ValidateInputsHandler on_validate_handler;

    public:
        explicit CreatedEventHandler(std::weak_ptr<Fusion360Command> command) :
            m_command{std::move( command )},
            on_destroy_handler{ CommandDestroyHandler(m_command) },
            on_execute_handler{ CommandExecuteHandler(m_command) },
            on_change_handler{ InputChangedHandler(m_command) },
            on_preview_handler{ ExecutePreviewHandler(m_command) },
            on_validate_handler{ ValidateInputsHandler(m_command) }
        {
        };
        void notify(const adsk::core::Ptr<adsk::core::CommandCreatedEventArgs>& args) override;
    };
    
    }

#endif /* silvanuspro_eventhandlers_hpp */
