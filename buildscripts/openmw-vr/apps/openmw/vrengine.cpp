#include "engine.hpp"

#include "mwbase/environment.hpp"

#include "mwvr/vrenvironment.hpp"

#include "mwvr/openxrmanager.hpp"
#include "mwvr/vrsession.hpp"
#include "mwvr/vrviewer.hpp"
#include "mwvr/vrgui.hpp"
#include "mwvr/vrtracking.hpp"

#include <components/debug/debuglog.hpp>

#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifndef USE_OPENXR
#error "USE_OPENXR not defined"
#endif

void OMW::Engine::initVr()
{
    if (!mViewer)
        throw std::logic_error("mViewer must be initialized before calling initVr()");

    try {
        auto& xrEnvironment = MWVR::Environment::get();
        xrEnvironment.setManager(new MWVR::OpenXRManager);
        xrEnvironment.setSession(new MWVR::VRSession());
        xrEnvironment.setViewer(new MWVR::VRViewer(mViewer));
        xrEnvironment.setTrackingManager(new MWVR::VRTrackingManager());
        Log(Debug::Info) << "OpenXR VR initialization successful";
    } catch (const std::exception& e) {
        Log(Debug::Error) << "OpenXR VR initialization failed: " << e.what();
        #ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_FATAL, "OpenMWXRDiag", "initVr() failed: %s", e.what());
        #endif
        throw;
    }
}
