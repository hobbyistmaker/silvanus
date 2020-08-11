//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/22/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//

#include <vector>

#include <Core/CoreAll.h>

#include "SilvanusPro.h"
#include "lib/generateboxcommand.hpp"

using namespace adsk::core;
using namespace adsk::fusion;

using namespace silvanus;

Ptr<Application> app; // NOLINT(cert-err58-cpp)
Ptr<UserInterface> ui; // NOLINT(cert-err58-cpp)

std::vector<std::unique_ptr<common::CommandButton>> buttons;

using std::make_unique;
using std::make_shared;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "IncompatibleTypes"
extern "C" XI_EXPORT bool run(const char* context)
{
	app = Application::get();
	if (!app)
		return false;

    ui = app->userInterface();
    if (!ui)
        return false;

    ui->messageBox("Silvanus Pro Addin starting.");

    auto active_product = Ptr<Product>{app->activeProduct()};

    buttons.push_back(
        make_unique<common::CommandButton>(
                app, make_shared<GenerateBoxCommand>(app)
        )
    );
    
    ui->messageBox("Silvanus Pro Addin Started.");

    adsk::autoTerminate(false);
    return true;
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma ide diagnostic ignored "IncompatibleTypes"
extern "C" XI_EXPORT bool stop(const char* context)
{
    for(auto&& button: buttons) {
        button->stop();
    }
    buttons.clear();
    
	if (ui)
	{
		ui->messageBox("Silvanus Pro Addin Stopped.");
		ui = nullptr;
	}

    adsk::terminate();
	return true;
}
#pragma clang diagnostic pop


#ifdef XI_WIN

#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hmodule, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

#endif // XI_WIN
