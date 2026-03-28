#include "openxrswapchainimage.hpp"
#include "openxrmanagerimpl.hpp"
#include "vrenvironment.hpp"
#include "vrframebuffer.hpp"

// The OpenXR SDK's platform headers assume we've included platform headers
#ifdef _WIN32
#include <Windows.h>
#include <objbase.h>

#ifdef XR_USE_GRAPHICS_API_D3D11
#include <d3d11.h>
#include <dxgi1_2.h>
#endif

#elif __ANDROID__
#include <jni.h>
#include <EGL/egl.h>
#include <android/log.h>

#elif __linux__
#include <X11/Xlib.h>
#include <GL/glx.h>
#undef None

#else
#error Unsupported platform
#endif

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_platform_defines.h>
#include <openxr/openxr_reflection.h>

#include <stdexcept>

#define GLERR if(auto err = glGetError() != GL_NO_ERROR) Log(Debug::Verbose) << __FILE__ << "." << __LINE__ << ": " << glGetError()

namespace MWVR {

    template<typename Image>
    class OpenXRSwapchainImageTemplate;

#ifdef XR_USE_GRAPHICS_API_OPENGL
    template<>
    class OpenXRSwapchainImageTemplate< XrSwapchainImageOpenGLKHR > : public OpenXRSwapchainImage
    {
    public:
        static constexpr XrStructureType XrType = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR;

    public:
        OpenXRSwapchainImageTemplate(osg::GraphicsContext* gc, XrSwapchainCreateInfo swapchainCreateInfo, const XrSwapchainImageOpenGLKHR& xrImage)
            : OpenXRSwapchainImage()
            , mXrImage(xrImage)
            , mBufferBits(0)
            , mFramebuffer(nullptr)
        {
            mFramebuffer.reset(new VRFramebuffer(gc->getState(), swapchainCreateInfo.width, swapchainCreateInfo.height, swapchainCreateInfo.sampleCount));
            if (swapchainCreateInfo.usageFlags & XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                mFramebuffer->setDepthBuffer(gc, mXrImage.image, false);
                mBufferBits = GL_DEPTH_BUFFER_BIT;
            }
            else
            {
                mFramebuffer->setColorBuffer(gc, mXrImage.image, false);
                mBufferBits = GL_COLOR_BUFFER_BIT;
            }
        }

        void blit(osg::GraphicsContext* gc, VRFramebuffer& readBuffer, int offset_x, int offset_y) override
        {
            mFramebuffer->bindFramebuffer(gc, GL_FRAMEBUFFER_EXT);
            readBuffer.blit(gc, offset_x, offset_y, offset_x + mFramebuffer->width(), offset_y + mFramebuffer->height(), 0, 0, mFramebuffer->width(), mFramebuffer->height(), mBufferBits, GL_NEAREST);

#ifdef __ANDROID__
            if (mBufferBits == GL_COLOR_BUFFER_BIT)
            {
                static unsigned int sSwapchainBlitDiagFrame = 0;
                ++sSwapchainBlitDiagFrame;
                const bool shouldLog = (sSwapchainBlitDiagFrame <= 10) || ((sSwapchainBlitDiagFrame % 300) == 0);
                if (shouldLog)
                {
                    GLint viewport[4] = { 0, 0, 0, 0 };
                    GLint drawFbo = 0;
                    glGetIntegerv(GL_VIEWPORT, viewport);
                    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &drawFbo);

                    const int sampleX = (mFramebuffer->width() > 0) ? (mFramebuffer->width() / 2) : 0;
                    const int sampleY = (mFramebuffer->height() > 0) ? (mFramebuffer->height() / 2) : 0;
                    unsigned char pixel[4] = { 0, 0, 0, 0 };
                    glReadPixels(sampleX, sampleY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
                    const GLenum readErr = glGetError();

                    __android_log_print(
                        ANDROID_LOG_WARN,
                        "OpenMWXRDiag",
                        "SwapchainImage::blit diag frame=%u tex=%u fbo=%d vp=%d,%d %dx%d sample(%d,%d)=%u,%u,%u,%u readErr=0x%x srcOffset=%d,%d dstSize=%dx%d",
                        sSwapchainBlitDiagFrame,
                        static_cast<unsigned int>(mXrImage.image),
                        drawFbo,
                        viewport[0],
                        viewport[1],
                        viewport[2],
                        viewport[3],
                        sampleX,
                        sampleY,
                        pixel[0],
                        pixel[1],
                        pixel[2],
                        pixel[3],
                        static_cast<unsigned int>(readErr),
                        offset_x,
                        offset_y,
                        mFramebuffer->width(),
                        mFramebuffer->height());
                }
            }
#endif
        }

        XrSwapchainImageOpenGLKHR mXrImage;
        uint32_t mBufferBits;
        std::unique_ptr<VRFramebuffer> mFramebuffer;
    };
#endif

#ifdef XR_USE_GRAPHICS_API_OPENGL_ES

// Native GLES3 function pointers, loaded once via eglGetProcAddress to bypass GL4ES.
// GL4ES doesn't track Meta-runtime-allocated textures so glFramebufferTexture2D through
// GL4ES silently leaves the FBO with no attachments (INCOMPLETE_MISSING_ATTACHMENT).
namespace {
    struct NativeGLES {
        typedef void     (*PFNGenFramebuffers)(GLsizei, GLuint*);
        typedef void     (*PFNDeleteFramebuffers)(GLsizei, const GLuint*);
        typedef void     (*PFNBindFramebuffer)(GLenum, GLuint);
        typedef void     (*PFNFramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint, GLint);
        typedef void     (*PFNFramebufferTextureLayer)(GLenum, GLenum, GLuint, GLint, GLint);
        typedef GLenum   (*PFNCheckFramebufferStatus)(GLenum);
        typedef void     (*PFNBlitFramebuffer)(GLint,GLint,GLint,GLint,GLint,GLint,GLint,GLint,GLbitfield,GLenum);
        typedef GLenum   (*PFNGetError)();
        typedef void     (*PFNGetIntegerv)(GLenum, GLint*);

        PFNGenFramebuffers         genFramebuffers = nullptr;
        PFNDeleteFramebuffers      deleteFramebuffers = nullptr;
        PFNBindFramebuffer         bindFramebuffer = nullptr;
        PFNFramebufferTexture2D    framebufferTexture2D = nullptr;
        PFNFramebufferTextureLayer framebufferTextureLayer = nullptr;
        PFNCheckFramebufferStatus  checkFramebufferStatus = nullptr;
        PFNBlitFramebuffer         blitFramebuffer = nullptr;
        PFNGetError                getError = nullptr;
        PFNGetIntegerv             getIntegerv = nullptr;

        bool init() {
            if (genFramebuffers) return true;
            genFramebuffers         = (PFNGenFramebuffers)        eglGetProcAddress("glGenFramebuffers");
            deleteFramebuffers      = (PFNDeleteFramebuffers)     eglGetProcAddress("glDeleteFramebuffers");
            bindFramebuffer         = (PFNBindFramebuffer)        eglGetProcAddress("glBindFramebuffer");
            framebufferTexture2D    = (PFNFramebufferTexture2D)   eglGetProcAddress("glFramebufferTexture2D");
            framebufferTextureLayer = (PFNFramebufferTextureLayer)eglGetProcAddress("glFramebufferTextureLayer");
            checkFramebufferStatus  = (PFNCheckFramebufferStatus) eglGetProcAddress("glCheckFramebufferStatus");
            blitFramebuffer         = (PFNBlitFramebuffer)        eglGetProcAddress("glBlitFramebuffer");
            getError                = (PFNGetError)               eglGetProcAddress("glGetError");
            getIntegerv             = (PFNGetIntegerv)            eglGetProcAddress("glGetIntegerv");
            return genFramebuffers != nullptr;
        }
    };
    static NativeGLES sNativeGLES;
}

    template<>
    class OpenXRSwapchainImageTemplate< XrSwapchainImageOpenGLESKHR > : public OpenXRSwapchainImage
    {
    public:
        static constexpr XrStructureType XrType = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR;

    public:
        OpenXRSwapchainImageTemplate(osg::GraphicsContext* gc, XrSwapchainCreateInfo swapchainCreateInfo, const XrSwapchainImageOpenGLESKHR& xrImage)
            : OpenXRSwapchainImage()
            , mXrImage(xrImage)
            , mBufferBits(0)
            , mWidth(swapchainCreateInfo.width)
            , mHeight(swapchainCreateInfo.height)
            , mNativeFBO(0)
        {
            if (!sNativeGLES.init())
            {
                __android_log_print(ANDROID_LOG_ERROR, "OpenMWXRDiag",
                    "GLES SwapchainImage: eglGetProcAddress failed to load GLES3 functions!");
                return;
            }

            if (swapchainCreateInfo.usageFlags & XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                mBufferBits = GL_DEPTH_BUFFER_BIT;
                // depth swapchains - no native FBO needed for depth (not blitted to)
                return;
            }
            mBufferBits = GL_COLOR_BUFFER_BIT;

            // Save the currently bound FBO so we can restore it
            GLint savedFBO = 0;
            sNativeGLES.getIntegerv(GL_FRAMEBUFFER_BINDING, &savedFBO);

            // Create a native GLES3 FBO, bypassing GL4ES
            sNativeGLES.genFramebuffers(1, &mNativeFBO);
            sNativeGLES.bindFramebuffer(GL_FRAMEBUFFER, mNativeFBO);

            // Clear any pending errors
            while (sNativeGLES.getError() != GL_NO_ERROR) {}

            // Try GL_TEXTURE_2D first (most common for arraySize=1)
            sNativeGLES.framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D, mXrImage.image, 0);
            GLenum err1 = sNativeGLES.getError();
            GLenum status1 = sNativeGLES.checkFramebufferStatus(GL_FRAMEBUFFER);

            if (status1 != GL_FRAMEBUFFER_COMPLETE)
            {
                // Detach and try as a 2D_ARRAY layer 0 (some runtimes return layered textures)
                sNativeGLES.framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_2D, 0, 0);  // detach
                while (sNativeGLES.getError() != GL_NO_ERROR) {}

                if (sNativeGLES.framebufferTextureLayer)
                {
                    sNativeGLES.framebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                        mXrImage.image, 0, 0);
                    GLenum err2 = sNativeGLES.getError();
                    GLenum status2 = sNativeGLES.checkFramebufferStatus(GL_FRAMEBUFFER);

                    __android_log_print(ANDROID_LOG_WARN, "OpenMWXRDiag",
                        "GLES native FBO init: tex=%u TEX2D err=0x%x status=0x%x LAYER err=0x%x status=0x%x fbo=%u",
                        mXrImage.image, err1, status1, err2, status2, mNativeFBO);

                    if (status2 != GL_FRAMEBUFFER_COMPLETE)
                    {
                        __android_log_print(ANDROID_LOG_ERROR, "OpenMWXRDiag",
                            "GLES native FBO: BOTH attachment methods failed for tex=%u!", mXrImage.image);
                    }
                }
                else
                {
                    __android_log_print(ANDROID_LOG_ERROR, "OpenMWXRDiag",
                        "GLES native FBO: TEX2D failed (err=0x%x status=0x%x) and no glFramebufferTextureLayer available",
                        err1, status1);
                }
            }
            else
            {
                __android_log_print(ANDROID_LOG_WARN, "OpenMWXRDiag",
                    "GLES native FBO init: tex=%u TEX2D OK fbo=%u", mXrImage.image, mNativeFBO);
            }

            // Restore previous FBO
            sNativeGLES.bindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(savedFBO));
        }

        ~OpenXRSwapchainImageTemplate()
        {
            if (mNativeFBO && sNativeGLES.deleteFramebuffers)
                sNativeGLES.deleteFramebuffers(1, &mNativeFBO);
        }

        void blit(osg::GraphicsContext* gc, VRFramebuffer& readBuffer, int offset_x, int offset_y) override
        {
            if (!mNativeFBO || mBufferBits != GL_COLOR_BUFFER_BIT)
                return;

            // Save currently bound FBO
            GLint savedFBO = 0;
            sNativeGLES.getIntegerv(GL_FRAMEBUFFER_BINDING, &savedFBO);

            // Bind read (source: gamma-resolve, a native GLES FBO) and draw (dest: XR swapchain)
            // readBuffer.framebufferId() returns the native GLES FBO ID (GL4ES uses native IDs)
            sNativeGLES.bindFramebuffer(GL_READ_FRAMEBUFFER, readBuffer.framebufferId());
            sNativeGLES.bindFramebuffer(GL_DRAW_FRAMEBUFFER, mNativeFBO);

            // Blit from the gamma-resolve region to the full swapchain image
            const int srcX0 = offset_x;
            const int srcY0 = offset_y;
            const int srcX1 = offset_x + static_cast<int>(mWidth);
            const int srcY1 = offset_y + static_cast<int>(mHeight);
            sNativeGLES.blitFramebuffer(srcX0, srcY0, srcX1, srcY1,
                                        0, 0, static_cast<int>(mWidth), static_cast<int>(mHeight),
                                        GL_COLOR_BUFFER_BIT, GL_NEAREST);

            static unsigned int sBlitFrame = 0;
            ++sBlitFrame;
            const bool shouldLog = (sBlitFrame <= 6) || ((sBlitFrame % 300) == 0);
            if (shouldLog)
            {
                GLenum blitErr = sNativeGLES.getError();
                // Read center pixel from the swapchain destination to confirm pixels arrived
                sNativeGLES.bindFramebuffer(GL_READ_FRAMEBUFFER, mNativeFBO);
                const int sampleX = static_cast<int>(mWidth) / 2;
                const int sampleY = static_cast<int>(mHeight) / 2;
                unsigned char pixel[4] = { 0, 0, 0, 0 };
                // Use glReadPixels (goes through GL4ES but that's fine for diagnostics)
                glReadPixels(sampleX, sampleY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
                GLenum readErr = sNativeGLES.getError();

                __android_log_print(ANDROID_LOG_WARN, "OpenMWXRDiag",
                    "GLES native blit #%u tex=%u fbo=%u blitErr=0x%x sample(%d,%d)=%u,%u,%u,%u readErr=0x%x srcOff=%d,%d size=%ux%u",
                    sBlitFrame, mXrImage.image, mNativeFBO,
                    static_cast<unsigned int>(blitErr),
                    sampleX, sampleY,
                    pixel[0], pixel[1], pixel[2], pixel[3],
                    static_cast<unsigned int>(readErr),
                    offset_x, offset_y,
                    mWidth, mHeight);
            }

            // Restore the FBO that was current before (GL4ES is unaffected — it tracks its own state)
            sNativeGLES.bindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(savedFBO));
        }

        XrSwapchainImageOpenGLESKHR mXrImage;
        uint32_t mBufferBits;
        uint32_t mWidth;
        uint32_t mHeight;
        GLuint   mNativeFBO;
    };
#endif

#ifdef XR_USE_GRAPHICS_API_D3D11
    template<>
    class OpenXRSwapchainImageTemplate< XrSwapchainImageD3D11KHR > : public OpenXRSwapchainImage
    {
    public:
        static constexpr XrStructureType XrType = XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR;
    public:
        OpenXRSwapchainImageTemplate(osg::GraphicsContext* gc, XrSwapchainCreateInfo swapchainCreateInfo, const XrSwapchainImageD3D11KHR& xrImage)
            : OpenXRSwapchainImage()
            , mXrImage(xrImage)
            , mBufferBits(0)
            , mFramebuffer(nullptr)
        {
            mXrImage.texture->GetDevice(&mDevice);
            mDevice->GetImmediateContext(&mDeviceContext);

            mXrImage.texture->GetDesc(&mDesc);

            glGenTextures(1, &mGlTextureName);

            auto* xr = Environment::get().getManager();
            //mDxResourceShareHandle = xr->impl().platform().DXRegisterObject(mXrImage.texture, mGlTextureName, GL_TEXTURE_2D, true, nullptr);

            if (!mDxResourceShareHandle)
            {
                // Some OpenXR runtimes return textures that cannot be directly shared.
                // So we need to make a redundant texture to use as an intermediary...
                mSharedTextureDesc.Width = mDesc.Width;
                mSharedTextureDesc.Height = mDesc.Height;
                mSharedTextureDesc.MipLevels = mDesc.MipLevels;
                mSharedTextureDesc.ArraySize = mDesc.ArraySize;
                mSharedTextureDesc.Format = static_cast<DXGI_FORMAT>(swapchainCreateInfo.format);
                mSharedTextureDesc.SampleDesc = mDesc.SampleDesc;
                mSharedTextureDesc.Usage = D3D11_USAGE_DEFAULT;
                mSharedTextureDesc.BindFlags = 0;
                mSharedTextureDesc.CPUAccessFlags = 0;
                mSharedTextureDesc.MiscFlags = 0;;

                mDevice->CreateTexture2D(&mSharedTextureDesc, nullptr, &mSharedTexture);
                mXrImage.texture->GetDesc(&mSharedTextureDesc);
                mDxResourceShareHandle = xr->impl().platform().DXRegisterObject(mSharedTexture, mGlTextureName, GL_TEXTURE_2D, true, nullptr);
            }

            // Set up shared texture as blit target
            mFramebuffer.reset(new VRFramebuffer(gc->getState(), swapchainCreateInfo.width, swapchainCreateInfo.height, swapchainCreateInfo.sampleCount));

            if (swapchainCreateInfo.usageFlags & XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                mFramebuffer->setDepthBuffer(gc, mGlTextureName, false);
                mBufferBits = GL_DEPTH_BUFFER_BIT;
            }
            else
            {
                mFramebuffer->setColorBuffer(gc, mGlTextureName, false);
                mBufferBits = GL_COLOR_BUFFER_BIT;
            }
        }

        ~OpenXRSwapchainImageTemplate()
        {
            auto* xr = Environment::get().getManager();
            if (mDxResourceShareHandle)
                xr->impl().platform().DXUnregisterObject(mDxResourceShareHandle);
            glDeleteTextures(1, &mGlTextureName);
        }

        void blit(osg::GraphicsContext* gc, VRFramebuffer& readBuffer, int offset_x, int offset_y) override
        {
            // Blit readBuffer into directx texture, while flipping the Y axis.
            auto* xr = Environment::get().getManager();
            xr->impl().platform().DXLockObject(mDxResourceShareHandle);
            mFramebuffer->bindFramebuffer(gc, GL_FRAMEBUFFER_EXT);
            readBuffer.blit(gc, offset_x, offset_y, offset_x + mFramebuffer->width(), offset_y + mFramebuffer->height(), 0, mFramebuffer->height(), mFramebuffer->width(), 0, mBufferBits, GL_NEAREST);
            xr->impl().platform().DXUnlockObject(mDxResourceShareHandle);
            

            // If the d3d11 texture couldn't be shared directly, blit it again.
            if (mSharedTexture)
            {
                mDeviceContext->CopyResource(mXrImage.texture, mSharedTexture);
            }
        }

        ID3D11Device* mDevice = nullptr;
        ID3D11DeviceContext* mDeviceContext = nullptr;
        D3D11_TEXTURE2D_DESC mDesc;
        D3D11_TEXTURE2D_DESC mSharedTextureDesc;
        ID3D11Texture2D* mSharedTexture = nullptr;
        uint32_t mGlTextureName = 0;
        void* mDxResourceShareHandle = nullptr;

        XrSwapchainImageD3D11KHR mXrImage;
        uint32_t mBufferBits;
        std::unique_ptr<VRFramebuffer> mFramebuffer;
    };
#endif


    template< typename Image > static inline
    std::vector<std::unique_ptr<OpenXRSwapchainImage> > 
        enumerateSwapchainImagesImpl(osg::GraphicsContext* gc, XrSwapchain swapchain, XrSwapchainCreateInfo swapchainCreateInfo)
    {
        using SwapchainImage = OpenXRSwapchainImageTemplate<Image>;

        uint32_t imageCount = 0;
        std::vector< Image > images;
        CHECK_XRCMD(xrEnumerateSwapchainImages(swapchain, 0, &imageCount, nullptr));
        images.resize(imageCount, { SwapchainImage::XrType });
        CHECK_XRCMD(xrEnumerateSwapchainImages(swapchain, imageCount, &imageCount, reinterpret_cast<XrSwapchainImageBaseHeader*>(images.data())));

        std::vector<std::unique_ptr<OpenXRSwapchainImage> > swapchainImages;
        for(auto& image: images)
        {
            swapchainImages.emplace_back(new OpenXRSwapchainImageTemplate<Image>(gc, swapchainCreateInfo, image));
        }

        return swapchainImages;
    }

    std::vector<std::unique_ptr<OpenXRSwapchainImage> > 
        OpenXRSwapchainImage::enumerateSwapchainImages(osg::GraphicsContext* gc, XrSwapchain swapchain, XrSwapchainCreateInfo swapchainCreateInfo)
    {
        auto* xr = Environment::get().getManager();

        if (false)
        {
            // placeholder - OpenGL/GLES extension checks below
        }
#ifdef XR_USE_GRAPHICS_API_OPENGL
        else if (xr->xrExtensionIsEnabled(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME))
        {
            return enumerateSwapchainImagesImpl<XrSwapchainImageOpenGLKHR>(gc, swapchain, swapchainCreateInfo);
        }
#endif
#ifdef XR_USE_GRAPHICS_API_OPENGL_ES
        else if (xr->xrExtensionIsEnabled(XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME))
        {
            return enumerateSwapchainImagesImpl<XrSwapchainImageOpenGLESKHR>(gc, swapchain, swapchainCreateInfo);
        }
#endif
#ifdef XR_USE_GRAPHICS_API_D3D11
        else if (xr->xrExtensionIsEnabled(XR_KHR_D3D11_ENABLE_EXTENSION_NAME))
        {
            return enumerateSwapchainImagesImpl<XrSwapchainImageD3D11KHR>(gc, swapchain, swapchainCreateInfo);
        }
#endif
        else
        {
            throw std::logic_error("Implementation missing for selected graphics API");
        }

        return std::vector<std::unique_ptr<OpenXRSwapchainImage>>();
    }

    OpenXRSwapchainImage::OpenXRSwapchainImage()
    {
    }
}
