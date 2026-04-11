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
#include <algorithm>
#include <cstring>
#include <vector>

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

        void blit(osg::GraphicsContext* gc, VRFramebuffer& readBuffer, int offset_x, int offset_y,
            const unsigned char*, int, int) override
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
        typedef void     (*PFNClearColor)(GLfloat, GLfloat, GLfloat, GLfloat);
        typedef void     (*PFNClear)(GLbitfield);
        typedef void     (*PFNReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*);
        typedef void     (*PFNBindTexture)(GLenum, GLuint);
        typedef void     (*PFNPixelStorei)(GLenum, GLint);
        typedef void     (*PFNTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*);

        PFNGenFramebuffers         genFramebuffers = nullptr;
        PFNDeleteFramebuffers      deleteFramebuffers = nullptr;
        PFNBindFramebuffer         bindFramebuffer = nullptr;
        PFNFramebufferTexture2D    framebufferTexture2D = nullptr;
        PFNFramebufferTextureLayer framebufferTextureLayer = nullptr;
        PFNCheckFramebufferStatus  checkFramebufferStatus = nullptr;
        PFNBlitFramebuffer         blitFramebuffer = nullptr;
        PFNGetError                getError = nullptr;
        PFNGetIntegerv             getIntegerv = nullptr;
        PFNClearColor              clearColor = nullptr;
        PFNClear                   clear = nullptr;
        PFNReadPixels              readPixels = nullptr;
        PFNBindTexture             bindTexture = nullptr;
        PFNPixelStorei             pixelStorei = nullptr;
        PFNTexSubImage2D           texSubImage2D = nullptr;

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
            clearColor              = (PFNClearColor)             eglGetProcAddress("glClearColor");
            clear                   = (PFNClear)                  eglGetProcAddress("glClear");
            readPixels              = (PFNReadPixels)             eglGetProcAddress("glReadPixels");
            bindTexture             = (PFNBindTexture)            eglGetProcAddress("glBindTexture");
            pixelStorei             = (PFNPixelStorei)            eglGetProcAddress("glPixelStorei");
            texSubImage2D           = (PFNTexSubImage2D)          eglGetProcAddress("glTexSubImage2D");
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
            , mNativeReadFBO(0)
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
            sNativeGLES.genFramebuffers(1, &mNativeReadFBO);
            sNativeGLES.bindFramebuffer(GL_FRAMEBUFFER, mNativeFBO);

            // Clear any pending errors
            while (sNativeGLES.getError() != GL_NO_ERROR) {}

            // Prefer layered attachment on Quest runtimes; they frequently expose swapchain
            // images as array-backed textures even with arraySize=1.
            GLenum errLayer = GL_INVALID_OPERATION;
            GLenum statusLayer = 0;
            if (sNativeGLES.framebufferTextureLayer)
            {
                sNativeGLES.framebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    mXrImage.image, 0, 0);
                errLayer = sNativeGLES.getError();
                statusLayer = sNativeGLES.checkFramebufferStatus(GL_FRAMEBUFFER);
            }

            if (statusLayer != GL_FRAMEBUFFER_COMPLETE)
            {
                // Layered attach failed; retry as classic 2D texture target.
                sNativeGLES.framebufferTextureLayer ? sNativeGLES.framebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0, 0)
                                                    : sNativeGLES.framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
                while (sNativeGLES.getError() != GL_NO_ERROR) {}

                sNativeGLES.framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_2D, mXrImage.image, 0);
                GLenum err2D = sNativeGLES.getError();
                GLenum status2D = sNativeGLES.checkFramebufferStatus(GL_FRAMEBUFFER);

                __android_log_print(ANDROID_LOG_WARN, "OpenMWXRDiag",
                    "GLES native FBO init: tex=%u LAYER err=0x%x status=0x%x TEX2D err=0x%x status=0x%x fbo=%u",
                    mXrImage.image,
                    static_cast<unsigned int>(errLayer),
                    static_cast<unsigned int>(statusLayer),
                    static_cast<unsigned int>(err2D),
                    static_cast<unsigned int>(status2D),
                    mNativeFBO);

                if (status2D != GL_FRAMEBUFFER_COMPLETE)
                {
                    __android_log_print(ANDROID_LOG_ERROR, "OpenMWXRDiag",
                        "GLES native FBO: BOTH attachment methods failed for tex=%u!", mXrImage.image);
                }
            }
            else
            {
                __android_log_print(ANDROID_LOG_WARN, "OpenMWXRDiag",
                    "GLES native FBO init: tex=%u LAYER OK fbo=%u", mXrImage.image, mNativeFBO);
            }

            // Restore previous FBO
            sNativeGLES.bindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(savedFBO));
        }

        ~OpenXRSwapchainImageTemplate()
        {
            if (mNativeFBO && sNativeGLES.deleteFramebuffers)
                sNativeGLES.deleteFramebuffers(1, &mNativeFBO);
            if (mNativeReadFBO && sNativeGLES.deleteFramebuffers)
                sNativeGLES.deleteFramebuffers(1, &mNativeReadFBO);
        }

        void blit(osg::GraphicsContext* gc, VRFramebuffer& readBuffer, int offset_x, int offset_y,
            const unsigned char* sourcePixels, int sourceWidth, int sourceHeight) override
        {
            if (!mNativeFBO || mBufferBits != GL_COLOR_BUFFER_BIT)
                return;

            const int width = static_cast<int>(mWidth);
            const int height = static_cast<int>(mHeight);
            const int pixelCount = width * height;
            if (pixelCount <= 0)
                return;

            static unsigned int sBlitFrame = 0;
            ++sBlitFrame;
            const bool shouldLog = (sBlitFrame <= 8) || ((sBlitFrame % 300) == 0);

            unsigned char* pixelData = new unsigned char[pixelCount * 4];
            GLenum srcStatus = GL_FRAMEBUFFER_COMPLETE;
            GLenum readErr = GL_NO_ERROR;
            auto* gl = osg::GLExtensions::Get(gc->getState()->getContextID(), true);

            const bool hasExternalPixels = sourcePixels != nullptr
                && sourceWidth >= (offset_x + width)
                && sourceHeight >= (offset_y + height)
                && offset_x >= 0
                && offset_y >= 0;

            bool useDirectReadFallback = false;
            if (hasExternalPixels)
            {
                for (int y = 0; y < height; ++y)
                {
                    const unsigned char* srcRow = sourcePixels
                        + ((offset_y + y) * sourceWidth + offset_x) * 4;
                    unsigned char* dstRow = pixelData + (y * width * 4);
                    std::memcpy(dstRow, srcRow, static_cast<size_t>(width) * 4u);
                }

                const int probeIdx = ((height / 2) * width + (width / 2)) * 4;
                const bool extLooksBlack = pixelData[probeIdx + 0] == 0
                    && pixelData[probeIdx + 1] == 0
                    && pixelData[probeIdx + 2] == 0;
                if (extLooksBlack)
                {
                    unsigned char probe[4] = { 0, 0, 0, 0 };
                    gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, readBuffer.framebufferId());
                    while (glGetError() != GL_NO_ERROR) {}
                    glReadPixels(offset_x + (width / 2), offset_y + (height / 2), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, probe);
                    const GLenum probeErr = glGetError();
                    if (probeErr == GL_NO_ERROR && (probe[0] != 0 || probe[1] != 0 || probe[2] != 0))
                        useDirectReadFallback = true;
                }
            }

            if (!hasExternalPixels || useDirectReadFallback)
            {
                gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, readBuffer.framebufferId());
                srcStatus = gl->glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
                if (srcStatus != GL_FRAMEBUFFER_COMPLETE)
                {
                    if (shouldLog)
                        __android_log_print(ANDROID_LOG_WARN, "OpenMWXRDiag",
                            "VRSwapchain copy #%u source incomplete status=0x%x",
                            sBlitFrame, static_cast<unsigned int>(srcStatus));
                    delete[] pixelData;
                    return;
                }

                constexpr int kTileW = 512;
                constexpr int kTileH = 128;
                std::vector<unsigned char> tile(static_cast<size_t>(kTileW) * static_cast<size_t>(kTileH) * 4u);

                for (int y = 0; y < height && readErr == GL_NO_ERROR; y += kTileH)
                {
                    const int tileH = std::min(kTileH, height - y);
                    for (int x = 0; x < width && readErr == GL_NO_ERROR; x += kTileW)
                    {
                        const int tileW = std::min(kTileW, width - x);
                        glReadPixels(offset_x + x, offset_y + y, tileW, tileH, GL_RGBA, GL_UNSIGNED_BYTE, tile.data());
                        readErr = glGetError();
                        if (readErr != GL_NO_ERROR)
                            break;

                        for (int row = 0; row < tileH; ++row)
                        {
                            unsigned char* dst = pixelData
                                + ((static_cast<size_t>(y + row) * static_cast<size_t>(width) + static_cast<size_t>(x)) * 4u);
                            const unsigned char* src = tile.data()
                                + ((static_cast<size_t>(row) * static_cast<size_t>(tileW)) * 4u);
                            std::memcpy(dst, src, static_cast<size_t>(tileW) * 4u);
                        }
                    }
                }
            }

            if (readErr != GL_NO_ERROR)
            {
                if (shouldLog)
                    __android_log_print(ANDROID_LOG_WARN, "OpenMWXRDiag",
                        "VRSwapchain copy #%u glReadPixels err=0x%x",
                        sBlitFrame, static_cast<unsigned int>(readErr));
                delete[] pixelData;
                return;
            }

            const int srcSampleX = width / 2;
            const int srcSampleY = height / 2;
            const int srcSampleIdx = ((srcSampleY * width) + srcSampleX) * 4;
            const unsigned char srcR = pixelData[srcSampleIdx + 0];
            const unsigned char srcG = pixelData[srcSampleIdx + 1];
            const unsigned char srcB = pixelData[srcSampleIdx + 2];
            const unsigned char srcA = pixelData[srcSampleIdx + 3];

            if (!sNativeGLES.bindTexture || !sNativeGLES.pixelStorei || !sNativeGLES.texSubImage2D)
            {
                delete[] pixelData;
                return;
            }

            GLint savedDrawFbo = 0;
            GLint savedReadFbo = 0;
            sNativeGLES.getIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &savedDrawFbo);
            sNativeGLES.getIntegerv(GL_READ_FRAMEBUFFER_BINDING, &savedReadFbo);

            sNativeGLES.bindFramebuffer(GL_FRAMEBUFFER, mNativeFBO);
            const GLenum dstStatus = sNativeGLES.checkFramebufferStatus(GL_FRAMEBUFFER);
            if (dstStatus != GL_FRAMEBUFFER_COMPLETE)
            {
                if (shouldLog)
                    __android_log_print(ANDROID_LOG_WARN, "OpenMWXRDiag",
                        "VRSwapchain copy #%u dest incomplete status=0x%x",
                        sBlitFrame, static_cast<unsigned int>(dstStatus));
                sNativeGLES.bindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(savedDrawFbo));
                sNativeGLES.bindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(savedReadFbo));
                delete[] pixelData;
                return;
            }

            sNativeGLES.bindTexture(GL_TEXTURE_2D, mXrImage.image);
            sNativeGLES.pixelStorei(GL_UNPACK_ALIGNMENT, 1);
            sNativeGLES.texSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
            const GLenum texSubErr = sNativeGLES.getError();

            if (shouldLog)
            {
                sNativeGLES.bindFramebuffer(GL_READ_FRAMEBUFFER, mNativeFBO);
                const int sampleX = width / 2;
                const int sampleY = height / 2;
                unsigned char sample[4] = { 0, 0, 0, 0 };
                if (sNativeGLES.readPixels)
                    sNativeGLES.readPixels(sampleX, sampleY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, sample);
                const GLenum verifyErr = sNativeGLES.getError();

                __android_log_print(ANDROID_LOG_WARN, "OpenMWXRDiag",
                    "VRSwapchain copy #%u tex=%u dstFbo=%u srcFbo=%u offs=%d,%d ext=%d src=0x%x dst=0x%x readErr=0x%x texErr=0x%x srcSample=%u,%u,%u,%u dstSample=%u,%u,%u,%u verifyErr=0x%x",
                    sBlitFrame,
                    mXrImage.image,
                    mNativeFBO,
                    readBuffer.framebufferId(),
                    offset_x,
                    offset_y,
                    hasExternalPixels ? 1 : 0,
                    static_cast<unsigned int>(srcStatus),
                    static_cast<unsigned int>(dstStatus),
                    static_cast<unsigned int>(readErr),
                    static_cast<unsigned int>(texSubErr),
                    srcR, srcG, srcB, srcA,
                    sample[0], sample[1], sample[2], sample[3],
                    static_cast<unsigned int>(verifyErr));
            }

            sNativeGLES.bindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(savedDrawFbo));
            sNativeGLES.bindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(savedReadFbo));
            delete[] pixelData;
        }

        XrSwapchainImageOpenGLESKHR mXrImage;
        uint32_t mBufferBits;
        uint32_t mWidth;
        uint32_t mHeight;
        GLuint   mNativeFBO;
        GLuint   mNativeReadFBO;
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

        void blit(osg::GraphicsContext* gc, VRFramebuffer& readBuffer, int offset_x, int offset_y,
            const unsigned char*, int, int) override
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
