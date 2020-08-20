//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/22/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//

#include <vector>

#include <Core/CoreAll.h>

#include <dlfcn.h>
#include <libgen.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path_traits.hpp>

#include "plog/Log.h"
#include "plog/Initializers/RollingFileInitializer.h"

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

    // MAC Only
    Dl_info dli;
    dladdr((void*)run, &dli);

    // Using boost::filesystem to support older MacOS
    auto fpath = boost::filesystem::path(dli.dli_fname);
    auto const bname = fpath.parent_path().append("silvanus.log");
    // -- MAC Only
    plog::init(plog::verbose, bname.c_str());

    auto active_product = Ptr<Product>{app->activeProduct()};

    buttons.emplace_back(
        make_unique<common::CommandButton>(
                app, make_shared<GenerateBoxCommand>(app)
        )
    );
    
    ui->messageBox("Silvanus Pro Addin Started.");
    PLOG_DEBUG << "Silvanus Pro Addin started.";

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

	PLOG_INFO << "Silvanus Pro Addin stopped.";
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
