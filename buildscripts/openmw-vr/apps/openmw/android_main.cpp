int stderr = 0; // Hack: fix linker error

#include "SDL_main.h"
#include <SDL_gamecontroller.h>
#include <SDL_mouse.h>
#include <SDL_events.h>

/*******************************************************************************
 Functions called by JNI
 *******************************************************************************/
#include <jni.h>
#include <EGL/egl.h>
#ifdef USE_OPENXR
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#endif
#include <string>

extern "C" JNIEXPORT void JNICALL Java_ui_activity_GameActivity_setOpenXrRuntimeJson(JNIEnv* env, jobject activity, jstring runtimeJsonPath);
extern "C" JNIEXPORT void JNICALL Java_ui_activity_GameActivity_initOpenXRLoader(JNIEnv* env, jobject activity);

// Captured in JNI_OnLoad; required for xrInitializeLoaderKHR on Android.
static JavaVM* gJavaVM = nullptr;

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    gJavaVM = vm;
    return JNI_VERSION_1_6;
}

/* Called before  to initialize JNI bindings  */

extern void SDL_Android_Init(JNIEnv* env, jclass cls);
extern int argcData;
extern const char **argvData;
void releaseArgv();


extern "C" int Java_org_libsdl_app_SDLActivity_getMouseX(JNIEnv *env, jclass cls, jobject obj) {
    int ret = 0;
    SDL_GetMouseState(&ret, nullptr);
    return ret;
}


extern "C" int Java_org_libsdl_app_SDLActivity_getMouseY(JNIEnv *env, jclass cls, jobject obj) {
    int ret = 0;
    SDL_GetMouseState(nullptr, &ret);
    return ret;
}

extern "C" int Java_org_libsdl_app_SDLActivity_isMouseShown(JNIEnv *env, jclass cls, jobject obj) {
    return SDL_ShowCursor(SDL_QUERY);
}

extern SDL_Window *Android_Window;
extern "C" int SDL_SendMouseMotion(SDL_Window * window, int mouseID, int relative, int x, int y);
extern "C" void Java_org_libsdl_app_SDLActivity_sendRelativeMouseMotion(JNIEnv *env, jclass cls, int x, int y) {
    SDL_SendMouseMotion(Android_Window, 0, 1, x, y);
}

extern "C" int SDL_SendMouseButton(SDL_Window * window, int mouseID, Uint8 state, Uint8 button);
extern "C" void Java_org_libsdl_app_SDLActivity_sendMouseButton(JNIEnv *env, jclass cls, int state, int button) {
    SDL_SendMouseButton(Android_Window, 0, state, button);
}

extern "C" int Java_org_libsdl_app_SDLActivity_nativeInit(JNIEnv* env, jclass cls, jobject obj) {
    setenv("OPENMW_DECOMPRESS_TEXTURES", "1", 1);

    // Keep these JNI entry points reachable so linker GC cannot discard them
    // from builds where they are only called via Java reflection-based lookup.
    volatile auto keepSetRuntime = &Java_ui_activity_GameActivity_setOpenXrRuntimeJson;
    volatile auto keepInitLoader = &Java_ui_activity_GameActivity_initOpenXRLoader;
    (void)keepSetRuntime;
    (void)keepInitLoader;

    // On Android, we use a virtual controller with guid="Virtual"
    SDL_GameControllerAddMapping("5669727475616c000000000000000000,Virtual,a:b0,b:b1,back:b15,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b16,leftshoulder:b6,leftstick:b13,lefttrigger:a5,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b14,righttrigger:a4,rightx:a2,righty:a3,start:b11,x:b3,y:b4");

    // Meta Quest Touch controllers (vendor 0x2833, product 0x0160).
    // Quest merges both controllers into a single Android InputDevice.
    // After axis deduplication the layout is:
    //   a0=leftx  a1=lefty  a2=rightx  a3=righty
    //   a4=lefttrigger  a5=righttrigger  a6=brake(=lt dup)  a7=gas(=rt dup)
    // GUID: bus=0x05(BT) vendor=0x2833 product=0x0160 btnmask=0x7fff axismask=0x003f
    SDL_GameControllerAddMapping("050000003328000060010000ff7f3f00,Meta Quest Touch Controller,a:b0,b:b1,x:b2,y:b3,back:b4,guide:b5,start:b6,leftstick:b7,rightstick:b8,leftshoulder:b9,rightshoulder:b10,dpup:b11,dpdown:b12,dpleft:b13,dpright:b14,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:a4,righttrigger:a5");

    return 0;
}

extern "C" JNIEXPORT void JNICALL Java_ui_activity_GameActivity_setOpenXrRuntimeJson(JNIEnv* env, jobject activity, jstring runtimeJsonPath) {
    (void)activity;
    if (!runtimeJsonPath) {
        return;
    }
    const char* pathChars = env->GetStringUTFChars(runtimeJsonPath, nullptr);
    if (!pathChars) {
        return;
    }
    std::string runtimePath(pathChars);
    env->ReleaseStringUTFChars(runtimeJsonPath, pathChars);
    if (!runtimePath.empty()) {
        setenv("XR_RUNTIME_JSON", runtimePath.c_str(), 1);
    }
}

// Global reference to the application context, needed by xrInitializeLoaderKHR
static jobject gApplicationContext = nullptr;

// Called from GameActivity.onCreate() before SDL_main starts to initialize the OpenXR loader.
// On Android, xrInitializeLoaderKHR must be called before any other OpenXR function so the
// loader can locate the Meta Quest OpenXR runtime.
extern "C" JNIEXPORT void JNICALL Java_ui_activity_GameActivity_initOpenXRLoader(JNIEnv* env, jobject activity) {
#ifndef USE_OPENXR
    (void)env;
    (void)activity;
    return;
#else
    if (!gJavaVM) {
        return;
    }
    
    // Get the Application context (not Activity context) for a more stable reference
    jclass activityClass = env->GetObjectClass(activity);
    jmethodID getApplicationContextMethod = env->GetMethodID(activityClass, "getApplicationContext", "()Landroid/content/Context;");
    jobject appContext = env->CallObjectMethod(activity, getApplicationContextMethod);
    
    // Create a global reference so it survives beyond this JNI call
    if (gApplicationContext) {
        env->DeleteGlobalRef(gApplicationContext);
    }
    gApplicationContext = env->NewGlobalRef(appContext);
    
    // Get the xrInitializeLoaderKHR function pointer
    PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR = nullptr;
    if (xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR",
            reinterpret_cast<PFN_xrVoidFunction*>(&xrInitializeLoaderKHR)) != XR_SUCCESS ||
            !xrInitializeLoaderKHR) {
        return;
    }
    XrLoaderInitInfoAndroidKHR loaderInitInfo{};
    loaderInitInfo.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
    loaderInitInfo.next = nullptr;
    loaderInitInfo.applicationVM = gJavaVM;
    loaderInitInfo.applicationContext = gApplicationContext;
    xrInitializeLoaderKHR(reinterpret_cast<const XrLoaderInitInfoBaseHeaderKHR*>(&loaderInitInfo));
#endif
}

// Surface lifecycle JNI callbacks.
// In the VR build, OpenXR manages swapchain surfaces directly.
// The Android surface lifecycle is handled by the OpenXR runtime,
// so these are no-ops.
extern "C" void Java_org_libsdl_app_SDLActivity_omwSurfaceDestroyed(JNIEnv *env, jclass cls, jobject obj) {
    // OpenXR session state machine handles background/minimise transitions
}

extern "C" void Java_org_libsdl_app_SDLActivity_omwSurfaceRecreated(JNIEnv *env, jclass cls, jobject obj) {
    // OpenXR session state machine handles surface restoration
}
