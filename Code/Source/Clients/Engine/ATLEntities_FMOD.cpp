#include "ATLEntities_FMOD.h"

//#include "Common_FMOD.h"
#include <AzCore/std/string/string.h>
#include <AzCore/StringFunc/StringFunc.h>

namespace AudioEngineFMOD
{

SATLAudioObjectData_FMOD::SATLAudioObjectData_FMOD(Audio::TAudioControlID id)
{

}


bool SATLAudioObjectData_FMOD::ApplyEvent(SATLEventData_FMOD *event)
{
    return false;
}


void SATLAudioObjectData_FMOD::RemoveEvent(SATLEventData_FMOD *event)
{

}


void SATLAudioObjectData_FMOD::StopEvent(uint32_t eventId)
{

}


void SATLAudioObjectData_FMOD::StopAllEvents()
{

}


void SATLAudioObjectData_FMOD::ApplyParameter(SATLParameterRtpc_FMOD *param)
{

}


void SATLAudioObjectData_FMOD::RemoveParameter(SATLParameterRtpc_FMOD *param)
{

}


void SATLAudioObjectData_FMOD::ApplySwitch(SATLSwitchStateImplData_FMOD *switchImpl)
{

}


void SATLAudioObjectData_FMOD::RemoveSwitch(SATLSwitchStateImplData_FMOD *switchImpl)
{

}


void SATLAudioObjectData_FMOD::ApplyEnvironment(SATLEnvironmentImplData_FMOD *env)
{

}


void SATLAudioObjectData_FMOD::RemoveEnvironment(SATLEnvironmentImplData_FMOD *env)
{

}

bool SATLEventData_FMOD::CheckOcclusionEnabled()
{
    return false;
}

void SATLEventData_FMOD::SetObstructionOcclusion(float obstruction, float occlusion)
{

}

void SATLEventData_FMOD::TrySetEnvironment(SATLEnvironmentImplData_FMOD *envData)
{

}

void SATLEventData_FMOD::Reset()
{

}

}