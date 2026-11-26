#include "ATLEntities_FMOD.h"

//#include "Common_FMOD.h"
#include <AzCore/std/string/string.h>
#include <AzCore/StringFunc/StringFunc.h>

namespace AudioEngineFMOD
{
    ATLAudioObject_FMOD::ATLAudioObject_FMOD(Audio::TAudioObjectID id)
        : m_id(id)
        , m_attributes({})
    {
        m_audioEvents.reserve(4);
    }

    bool ATLAudioObject_FMOD::PostAudioEvent(ATLEventData_FMOD* event)
    {
        bool suceess = false;
        if (event->PrepareForOcclusion())
        {

        }
        return suceess;
    }

    void ATLAudioObject_FMOD::UnpostAudioEvent(ATLEventData_FMOD* event)
    {
        if (auto it = AZStd::find(m_audioEvents.begin(), m_audioEvents.end(), event);
            it != m_audioEvents.end())
        {
            m_audioEvents.erase(it);
        }
    }

    bool ATLAudioObject_FMOD::PostParameter(ATLParameterRtpc_FMOD* rtpc)
    {
        return false;
    }

    void ATLAudioObject_FMOD::UnpostParameter(ATLParameterRtpc_FMOD* rtpc)
    {
    }

    void ATLAudioObject_FMOD::SetSwitch(ATLSwitchState_FMOD* state)
    {
    }

    void ATLAudioObject_FMOD::SetEnvironment(ATLEnvironment_FMOD* env)
    {
    }

    void ATLAudioObject_FMOD::Set3DAttributes(Audio::SATLWorldPosition& pos)
    {
    }

    void ATLAudioObject_FMOD::StopAllEvents()
    {
    }

    void ATLAudioObject_FMOD::StopPlayingEvent(uint32_t eventPathId)
    {
    }

    void ATLAudioObject_FMOD::SetObstructionOcclusion(float obstruction, float occlusion)
    {
    }

    void ATLAudioObject_FMOD::Reset()
    {
    }

    void ATLEventData_FMOD::Reset()
    {
    }

    bool ATLEventData_FMOD::PrepareForOcclusion()
    {
        m_masterTrack = nullptr;
        FMOD_RESULT result = m_instance->getChannelGroup(&m_masterTrack);

        if (m_masterTrack != nullptr && result == FMOD_OK)
        {

        }
        return false;
    }

    void ATLEventData_FMOD::SetOcclusionObstruction(float occlusion, float obstruction)
    {
    }

    void ATLEventData_FMOD::TrySetEnvironment(ATLEnvironment_FMOD* environment, float value)
    {
    }
}
