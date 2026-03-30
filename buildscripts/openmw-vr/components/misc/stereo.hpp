#ifndef OPENMW_COMPONENTS_MISC_STEREO_H
#define OPENMW_COMPONENTS_MISC_STEREO_H

#include <components/stereo/stereomanager.hpp>
#include <components/stereo/types.hpp>

#include <memory>

#include <osg/Camera>
#include <osg/NodeCallback>

namespace Misc
{
    using Pose = Stereo::Pose;
    using FieldOfView = Stereo::FieldOfView;
    using View = Stereo::View;

    class StereoView
    {
    public:
        using UpdateViewCallback = Stereo::Manager::UpdateViewCallback;

        struct StereoDrawCallback : public osg::Camera::DrawCallback
        {
            enum class View
            {
                Left,
                Right,
            };

            virtual void operator()(osg::RenderInfo& info, View view) const = 0;
        };

        static StereoView& instance()
        {
            static StereoView sInstance;
            return sInstance;
        }

        void setUpdateViewCallback(std::shared_ptr<UpdateViewCallback> cb)
        {
            Stereo::Manager::instance().setUpdateViewCallback(std::move(cb));
        }

        void setCullMask(unsigned int)
        {
        }

    private:
        StereoView() = default;
    };
}

#endif
