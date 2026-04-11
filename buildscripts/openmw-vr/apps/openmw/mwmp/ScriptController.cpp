#include <components/openmw-mp/Base/BaseStructs.hpp>

#include "../mwscript/interpretercontext.hpp"

#include "ScriptController.hpp"

unsigned short ScriptController::getPacketOriginFromContextType(unsigned short contextType)
{
    (void)contextType;
    return mwmp::CLIENT_GAMEPLAY;
}
