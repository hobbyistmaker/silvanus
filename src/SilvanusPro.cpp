//
//  SilvanusPro
//
//  Created by Hobbyist Maker on 7/22/20.
//  Copyright © 2020 HobbyistMaker. All rights reserved.
//

#include <vector>

#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>

#ifdef __APPLE__
    #include <dlfcn.h>
    #include <libgen.h>

    #include <boost/filesystem.hpp>
    #include <boost/filesystem/path_traits.hpp>
#elif defined _WIN32 || defined _WIN64
    #include <windows.h>
    #include <boost/filesystem.hpp>
    #include <boost/filesystem/path_traits.hpp>
#endif

#include "plog/Log.h"

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

#if defined __WIN32 || defined _WIN64
std::string dllPath()
{
    //char path[MAX_PATH];
    //HMODULE hm = NULL;

    //if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
    //    GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
    //    (LPCSTR)&dllPath, &hm) == 0)
    //{
    //    int ret = GetLastError();
    //    ui->messageBox("Error finding DLL: " + std::to_string(ret));
    //}
    //if (GetModuleFileNameA(hm, path, sizeof(path)) == 0) {
    //    int ret = GetLastError();
    //    ui->messageBox("Error finding DLL file name: " + std::to_string(ret));
    //}
    //auto fpath = boost::filesystem::path(path);
    //// Using boost::filesystem to support older MacOS
    //return fpath.parent_path().append("silvanus.log").string();
    return "";
}
#endif

#ifdef __APPLE__
#pragma clang diagnostic push
#pragma ide diagnostic ignored "IncompatibleTypes"
#endif
extern "C" XI_EXPORT bool run(const char* context)
{
	app = Application::get();
	if (!app)
		return false;

    ui = app->userInterface();
    if (!ui)
        return false;

    ui->messageBox("Silvanus Pro Addin Starting.");

#ifdef __APPLE__
    // MAC Only
    Dl_info dli;
    dladdr((void*)run, &dli);

    auto fpath = boost::filesystem::path(dli.dli_fname);

    // Using boost::filesystem to support older MacOS
    auto const bname = fpath.parent_path().append("silvanus.log");
    // -- MAC Only
    plog::init(plog::verbose, bname.c_str(), 52428800, 1); 
#elif defined _WIN32 || defined _WIN64
    //auto bname = dllPath();
    // -- MAC Only
    //plog::init(plog::verbose, bname.c_str(), 52428800, 1);
#endif

    auto active_product = Ptr<Product>{app->activeProduct()};

    buttons.emplace_back(
        make_unique<common::CommandButton>(
                app, make_shared<GenerateBoxCommand>(app)
        )
    );
    
    PLOG_DEBUG << "Silvanus Pro Addin started.";

    adsk::autoTerminate(false);
    return true;
}
#ifdef __APPLE__
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma ide diagnostic ignored "IncompatibleTypes"
#endif
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
#ifdef __APPLE__
#pragma clang diagnostic pop
#endif

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
