#include "vrutil.hpp"
#include "vrenvironment.hpp"
#include "vrtracking.hpp"
#include "vranimation.hpp"

#include <cmath>

#include "../mwbase/environment.hpp"
#include "../mwbase/windowmanager.hpp"
#include "../mwbase/world.hpp"

#include <components/misc/constants.hpp>

#include <components/esm3/loadmgef.hpp>

#include "../mwworld/class.hpp"
#include "../mwmechanics/creaturestats.hpp"
#include "../mwrender/renderingmanager.hpp"

#include "osg/Transform"

namespace MWVR
{
    namespace Util
    {
        std::pair<MWWorld::Ptr, osg::Vec3f> getHitContact(float distance, std::vector<MWWorld::Ptr>& targets)
        {
            return std::pair<MWWorld::Ptr, osg::Vec3f>();
        }

        std::pair<MWWorld::Ptr, float> getTouchTarget()
        {
            MWRender::RenderingManager::RayResult result;
            auto* tm = Environment::get().getTrackingManager();
            VRPath rightHandPath = tm->stringToVRPath("/user/hand/right/input/aim/pose");
            auto* source = tm->getSource("pcworld");
            auto distance = getPoseTarget(result, source->getTrackingPose(0, rightHandPath).pose, true);
            return std::pair<MWWorld::Ptr, float>(result.mHitObject, distance);
        }

        std::pair<MWWorld::Ptr, float> getWeaponTarget()
        {
            auto* anim = MWVR::Environment::get().getPlayerAnimation();

            MWRender::RenderingManager::RayResult result;
            auto distance = getPoseTarget(result, getNodePose(anim->getNode("weapon bone")), false);
            return std::pair<MWWorld::Ptr, float>(result.mHitObject, distance);
        }

        float getPoseTarget(MWRender::RenderingManager::RayResult& result, const Pose& pose, bool allowTelekinesis)
        {
            auto wm = MWBase::Environment::get().getWindowManager();
            auto world = MWBase::Environment::get().getWorld();
            auto maxDistance = world->getMaxActivationDistance();

            if (wm->isGuiMode() && wm->isConsoleMode())
                maxDistance *= 50.f;
            else
            {
                if (allowTelekinesis)
                {
                    static const int unitsPerFoot = std::ceil(Constants::UnitsPerFoot);
                    const auto player = world->getPlayerPtr();
                    const auto telekinesisRangeBonus
                        = player.getClass()
                              .getCreatureStats(player)
                              .getMagicEffects()
                              .getOrDefault(ESM::MagicEffect::Telekinesis)
                              .getMagnitude()
                        * unitsPerFoot;
                    maxDistance += telekinesisRangeBonus;
                }
            }

            const osg::Vec3f direction = pose.orientation * osg::Vec3f(0.f, 1.f, 0.f);
            result = world->getRenderingManager()->castRay(pose.position, pose.position + direction * maxDistance, true);

            float distance = 0.f;
            if (result.mHit)
                distance = result.mRatio * maxDistance;

            if (!result.mHitObject.isEmpty() && !result.mHitObject.getClass().allowTelekinesis(result.mHitObject)
                && distance > world->getMaxActivationDistance() && !wm->isGuiMode())
            {
                result.mHit = false;
                result.mHitObject = nullptr;
                distance = 0.f;
            }

            return distance;
        }

        Pose getNodePose(const osg::Node* node)
        {
            osg::Matrix worldMatrix = osg::computeLocalToWorld(node->getParentalNodePaths()[0]);
            Pose pose;
            pose.position = worldMatrix.getTrans();
            pose.orientation = worldMatrix.getRotate();
            return pose;
        }
    }
}
