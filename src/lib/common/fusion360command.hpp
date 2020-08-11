//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/21/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//

#ifndef SILVANUSPRO_FUSION360COMMAND_HPP
#define SILVANUSPRO_FUSION360COMMAND_HPP

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#define DESC             "Run a Fusion 360 Command Button"
#define ID               "DefaultFusion360CommandButton"
#define NAME             "Fusion 360 Command Button"
#define RESOURCES_FOLDER "resources"
#define PANEL_LOCATION   "SolidScriptsAddinsPanel"
#define PREVIEW_FLAG     "previewEnabledId"
#define DIALOG           "Start Dialog Box"

namespace silvanus::common {
    
        class Fusion360Command
        {
            bool dirty = false;
            bool valid = false;

            const std::string description = DESC;
            const std::string id = ID;
            const std::string name = NAME;
            const std::string resources_folder = RESOURCES_FOLDER;
            const std::string panel_location = PANEL_LOCATION;
            const std::string preview_flag = PREVIEW_FLAG;
            const std::string dialog = DIALOG;
            
        protected:
            adsk::core::Ptr<adsk::core::Application> m_app;

        public:
            explicit Fusion360Command(
                 const adsk::core::Ptr<adsk::core::Application> app
            );
            
            virtual bool onChange(const adsk::core::Ptr<adsk::core::InputChangedEventArgs>& args) { return true; };
            virtual void onCreate(const adsk::core::Ptr<adsk::core::CommandCreatedEventArgs>& args) {};
            virtual void onDestroy(const adsk::core::Ptr<adsk::core::CommandEventArgs>& args) {};
            virtual void onDeactivate(const adsk::core::Ptr<adsk::core::CommandEventArgs>& args) {};
            virtual void onExecute(const adsk::core::Ptr<adsk::core::CommandEventArgs>& args) = 0;
            virtual void onPreview(const adsk::core::Ptr<adsk::core::CommandEventArgs>& args) {};
            virtual bool onValidate(const adsk::core::Ptr<adsk::core::ValidateInputsEventArgs>& args) { return true; };
            
            virtual void postValidateValid(const adsk::core::Ptr<adsk::core::CommandEventArgs>& args) {};
            
            virtual void change(const adsk::core::Ptr<adsk::core::InputChangedEventArgs>& args);
            virtual void create(const adsk::core::Ptr<adsk::core::CommandCreatedEventArgs>& args);
            virtual void destroy(const adsk::core::Ptr<adsk::core::CommandEventArgs>& args);
            virtual void deactivate(const adsk::core::Ptr<adsk::core::CommandEventArgs>& args);
            virtual void execute(const adsk::core::Ptr<adsk::core::CommandEventArgs>& args);
            virtual bool preview(const adsk::core::Ptr<adsk::core::CommandEventArgs>& args);
            virtual bool validate(const adsk::core::Ptr<adsk::core::ValidateInputsEventArgs>& args);
            
            virtual std::string getDescription() { return description; };
            virtual std::string getId() { return id; };
            virtual std::string getName() { return name; };
            virtual std::string getResourceFolder() { return resources_folder; };
            virtual std::string getPanelLocation() { return panel_location; };
            virtual std::string getPreviewFlag() { return preview_flag; };
            virtual std::string getDialogName() { return dialog; };
        };
    
    }

#endif /* silvanuspro_fusion360command_hpp */
