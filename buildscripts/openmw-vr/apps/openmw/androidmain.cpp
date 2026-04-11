#ifndef stderr
int stderr = 0; // Hack: fix linker error
#endif

#include "SDL_main.h"
#include <SDL_events.h>
#include <SDL_gamecontroller.h>
#include <SDL_mouse.h>
#include <EGL/egl.h>
#include <jni.h>
#ifdef USE_OPENXR
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#endif
#include <string>

/*******************************************************************************
 Functions called by JNI
 *******************************************************************************/

extern "C" JNIEXPORT void JNICALL Java_ui_activity_GameActivity_setOpenXrRuntimeJson(JNIEnv* env, jobject activity, jstring runtimeJsonPath);
extern "C" JNIEXPORT void JNICALL Java_ui_activity_GameActivity_initOpenXRLoader(JNIEnv* env, jobject activity);

// Captured in JNI_OnLoad; required for xrInitializeLoaderKHR on Android.
static JavaVM* gJavaVM = nullptr;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    gJavaVM = vm;
    return JNI_VERSION_1_6;
}

/* Called before  to initialize JNI bindings  */

extern void SDL_Android_Init(JNIEnv* env, jclass cls);
extern int argcData;
extern const char** argvData;
void releaseArgv();

extern "C" int Java_org_libsdl_app_SDLActivity_getMouseX(JNIEnv* env, jclass cls, jobject obj)
{
    int ret = 0;
    SDL_GetMouseState(&ret, nullptr);
    return ret;
}

extern "C" int Java_org_libsdl_app_SDLActivity_getMouseY(JNIEnv* env, jclass cls, jobject obj)
{
    int ret = 0;
    SDL_GetMouseState(nullptr, &ret);
    return ret;
}

extern "C" int Java_org_libsdl_app_SDLActivity_isMouseShown(JNIEnv* env, jclass cls, jobject obj)
{
    return SDL_ShowCursor(SDL_QUERY);
}

extern SDL_Window* Android_Window;
extern "C" int SDL_SendMouseMotion(SDL_Window* window, int mouseID, int relative, int x, int y);
extern "C" void Java_org_libsdl_app_SDLActivity_sendRelativeMouseMotion(JNIEnv* env, jclass cls, int x, int y)
{
    SDL_SendMouseMotion(Android_Window, 0, 1, x, y);
}

extern "C" int SDL_SendMouseButton(SDL_Window* window, int mouseID, Uint8 state, Uint8 button);
extern "C" void Java_org_libsdl_app_SDLActivity_sendMouseButton(JNIEnv* env, jclass cls, int state, int button)
{
    SDL_SendMouseButton(Android_Window, 0, state, button);
}

extern "C" int Java_org_libsdl_app_SDLActivity_nativeInit(JNIEnv* env, jclass cls, jobject obj)
{
    setenv("OPENMW_DECOMPRESS_TEXTURES", "1", 1);

    // Keep these JNI entry points reachable so linker GC cannot discard them
    // from builds where they are only called via Java reflection-based lookup.
    volatile auto keepSetRuntime = &Java_ui_activity_GameActivity_setOpenXrRuntimeJson;
    volatile auto keepInitLoader = &Java_ui_activity_GameActivity_initOpenXRLoader;
    (void)keepSetRuntime;
    (void)keepInitLoader;

    // On Android, we use a virtual controller with guid="Virtual"
    SDL_GameControllerAddMapping(
        "5669727475616c000000000000000000,Virtual,a:b0,b:b1,back:b15,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,"
        "guide:b16,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,"
        "righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4");

    return 0;
}

extern "C" JNIEXPORT void JNICALL Java_ui_activity_GameActivity_setOpenXrRuntimeJson(JNIEnv* env, jobject activity, jstring runtimeJsonPath)
{
    (void)activity;
    if (!runtimeJsonPath)
        return;

    const char* pathChars = env->GetStringUTFChars(runtimeJsonPath, nullptr);
    if (!pathChars)
        return;

    std::string runtimePath(pathChars);
    env->ReleaseStringUTFChars(runtimeJsonPath, pathChars);
    if (!runtimePath.empty())
        setenv("XR_RUNTIME_JSON", runtimePath.c_str(), 1);
}

// Global reference to the application context, needed by xrInitializeLoaderKHR.
static jobject gApplicationContext = nullptr;

extern "C" JNIEXPORT void JNICALL Java_ui_activity_GameActivity_initOpenXRLoader(JNIEnv* env, jobject activity)
{
#ifndef USE_OPENXR
    (void)env;
    (void)activity;
    return;
#else
    if (!gJavaVM)
        return;

    jclass activityClass = env->GetObjectClass(activity);
    jmethodID getApplicationContextMethod = env->GetMethodID(activityClass, "getApplicationContext", "()Landroid/content/Context;");
    jobject appContext = env->CallObjectMethod(activity, getApplicationContextMethod);

    if (gApplicationContext)
        env->DeleteGlobalRef(gApplicationContext);
    gApplicationContext = env->NewGlobalRef(appContext);

    PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR = nullptr;
    if (xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR",
            reinterpret_cast<PFN_xrVoidFunction*>(&xrInitializeLoaderKHR)) != XR_SUCCESS ||
        !xrInitializeLoaderKHR)
        return;

    XrLoaderInitInfoAndroidKHR loaderInitInfo{};
    loaderInitInfo.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
    loaderInitInfo.next = nullptr;
    loaderInitInfo.applicationVM = gJavaVM;
    loaderInitInfo.applicationContext = gApplicationContext;
    xrInitializeLoaderKHR(reinterpret_cast<const XrLoaderInitInfoBaseHeaderKHR*>(&loaderInitInfo));
#endif
}

extern "C" void Java_org_libsdl_app_SDLActivity_omwSurfaceDestroyed(JNIEnv* env, jclass cls, jobject obj)
{
    // OpenXR session state machine handles background/minimise transitions.
}

extern "C" void Java_org_libsdl_app_SDLActivity_omwSurfaceRecreated(JNIEnv* env, jclass cls, jobject obj)
{
    // OpenXR session state machine handles surface restoration.
}
