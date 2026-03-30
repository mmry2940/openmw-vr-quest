#include "vrcamera.hpp"
#include "vrgui.hpp"
#include "vrinputmanager.hpp"
#include "vrenvironment.hpp"
#include "vranimation.hpp"

#include <components/misc/constants.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/world.hpp"
#include "../mwbase/windowmanager.hpp"

#include "../mwworld/player.hpp"
#include "../mwworld/class.hpp"

#include "../mwmechanics/movement.hpp"

#include <osg/Quat>

namespace MWVR
{

    VRCamera::VRCamera(osg::Camera* camera)
        : MWRender::Camera(camera)
    {
        setMode(MWRender::Camera::Mode::FirstPerson, true);

        auto vrTrackingManager = MWVR::Environment::get().getTrackingManager();
        vrTrackingManager->bind(this, "pcworld");
    }

    VRCamera::~VRCamera()
    {
    }

    void VRCamera::setShouldTrackPlayerCharacter(bool track)
    {
        mShouldTrackPlayerCharacter = track;
    }

    void VRCamera::recenter()
    {
        if (!mHasTrackingData)
            return;

        // Move position of head to center of character 
        // Z should not be affected

        auto* session = Environment::get().getSession();

        auto* tm = Environment::get().getTrackingManager();
        auto* ws = static_cast<VRTrackingToWorldBinding*>(tm->getSource("pcworld"));


        ws->setSeatedPlay(session->seatedPlay());
        ws->setEyeLevel(session->eyeLevel() * Constants::UnitsPerMeter);
        ws->recenter(mShouldResetZ);



        mShouldRecenter = false;
        Log(Debug::Verbose) << "Recentered";
    }

    void VRCamera::applyTracking()
    {
        MWBase::World* world = MWBase::Environment::get().getWorld();

        auto& player = world->getPlayer();
        auto playerPtr = player.getPlayer();

        float yaw = 0.f;
        float pitch = 0.f;
        float roll = 0.f;
        getEulerAngles(mHeadPose.orientation, yaw, pitch, roll);

        world->rotateObject(playerPtr, osg::Vec3f(pitch, 0.f, yaw), MWBase::RotationFlag_none);
    }

    void VRCamera::onTrackingUpdated(VRTrackingSource& source, DisplayTime predictedDisplayTime)
    {
        auto path = Environment::get().getTrackingManager()->stringToVRPath("/user/head/input/pose");
        auto tp = source.getTrackingPose(predictedDisplayTime, path);

        if (!!tp.status)
        {
            mHeadPose = tp.pose;
            mHasTrackingData = true;
        }

        if (mShouldRecenter)
        {
            recenter();
            auto* vrGuiManager = MWVR::Environment::get().getGUIManager();
            vrGuiManager->updateTracking();
        }
        else
        {
            if (mShouldTrackPlayerCharacter && !MWBase::Environment::get().getWindowManager()->isGuiMode())
                applyTracking();
        }
    }

    void VRCamera::updateCamera(osg::Camera* cam)
    {
        // The regular update call should do nothing while tracking the player
    }

    void VRCamera::updateCamera()
    {
        // Base Camera callback updates the render camera each frame.
    }

    void VRCamera::reset()
    {
        Camera::reset();
    }

    void VRCamera::rotateCamera(float pitch, float roll, float yaw, bool adjust)
    {
        if (adjust)
        {
            pitch += getPitch();
            yaw += getYaw();
        }
        setYaw(yaw);
        setPitch(pitch);
    }

    void VRCamera::toggleViewMode(bool force)
    {
        setMode(MWRender::Camera::Mode::FirstPerson, true);
    }
    bool VRCamera::toggleVanityMode(bool enable)
    {
        // Vanity mode makes no sense in VR
        return !enable;
    }
    void VRCamera::allowVanityMode(bool allow)
    {
        // Vanity mode remains disabled for VR.
        (void)allow;
    }
    void VRCamera::getPosition(osg::Vec3d& focal, osg::Vec3d& camera) const
    {
        camera = focal = mHeadPose.position;
    }
    void VRCamera::getOrientation(osg::Quat& orientation) const
    {
        orientation = mHeadPose.orientation;
    }

    void VRCamera::processViewChange()
    {
        Camera::processViewChange();
    }

    void VRCamera::instantTransition()
    {
        Camera::instantTransition();

        // When the cell changes, openmw rotates the character.
        // To make sure the player faces the same direction regardless of current orientation,
        // compute the offset from character orientation to player orientation and reset yaw offset to this.
        float yaw = 0.f;
        float pitch = 0.f;
        float roll = 0.f;
        getEulerAngles(mHeadPose.orientation, yaw, pitch, roll);
        yaw = -getYaw() - yaw;
        auto* tm = Environment::get().getTrackingManager();
        auto* ws = static_cast<VRTrackingToWorldBinding*>(tm->getSource("pcworld"));
        ws->setWorldOrientation(yaw, true);
    }

    void VRCamera::rotateStage(float yaw)
    {
        auto* tm = Environment::get().getTrackingManager();
        auto* ws = static_cast<VRTrackingToWorldBinding*>(tm->getSource("pcworld"));
        ws->setWorldOrientation(yaw, true);
    }

    osg::Quat VRCamera::stageRotation()
    {
        auto* tm = Environment::get().getTrackingManager();
        auto* ws = static_cast<VRTrackingToWorldBinding*>(tm->getSource("pcworld"));
        return ws->getWorldOrientation();
    }

    void VRCamera::requestRecenter(bool resetZ)
    {
        mShouldRecenter = true;

        // Use OR so we don't negate a pending requests.
        mShouldResetZ |= resetZ;
    }
}
