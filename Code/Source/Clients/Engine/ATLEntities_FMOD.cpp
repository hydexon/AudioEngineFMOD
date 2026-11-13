#include "ATLEntities_FMOD.h"
#include "IAudioSystem.h"

//#include "Common_FMOD.h"
#include <AzCore/std/string/string.h>
#include <AzCore/StringFunc/StringFunc.h>

namespace AudioEngineFMOD
{

SATLAudioObjectData_FMOD::SATLAudioObjectData_FMOD(Audio::TAudioControlID id)
    : m_id(id)
    , m_attributes({})
{
    m_events.reserve(4);
}

void SATLAudioObjectData_FMOD::Reset()
{
    m_events.clear();
    m_params.clear();
    m_switches.clear();
}

void SATLAudioObjectData_FMOD::Set3DAttributes(const Audio::SATLWorldPosition &attribs)
{
    m_attributes = Utils::CreateFMOD3DAttributes(attribs.GetPositionVec(), attribs.GetForwardVec(), attribs.GetUpVec());
    for(auto event : m_events)
    {
        event->m_evtInstance->set3DAttributes(&m_attributes);
    }
}

void SATLAudioObjectData_FMOD::SetObstructionAndOcclusion(float obstruction, float occlusion)
{
    for(auto const event : m_events)
    {
        event->SetObstructionOcclusion(obstruction, occlusion);
    }
}


bool SATLAudioObjectData_FMOD::ApplyEvent(SATLEventData_FMOD *event)
{
    if(event->PrepareForOcclusion())
    {
        m_events.push_back(event);

        for(auto param : m_params)
        {
            auto it = g_ParameterToID.find(param.first);
            if(it != g_ParameterToID.end())
            {
                if(event->m_eventPathId == it->first->m_eventPathId)
                {
                    if(it->second.valid)
                    {
                        event->m_evtInstance->setParameterByID(it->second.id, param.second);
                    }
                }
                else
                {
                    ApplyParameter(it->first, param.second);
                }
            }
        }

        for(auto switchPair : m_switches)
        {
            auto it = g_SwitchToID.find(switchPair.second);
            if(it != g_SwitchToID.end())
            {
                if(it->second.valid)
                {
                    event->m_evtInstance->setParameterByID(it->second.id, switchPair.second->value);
                }
                else
                {
                    ApplySwitch(switchPair.second);
                }
            }
        }

        for(auto env : m_environments)
        {
            event->TrySetEnvironment(env.first, env.second);
        }

        FMOD_RESULT res = event->m_evtInstance->start();
        AZ_Assert(res == FMOD_OK, "Failed to start instance");
        return true;
    }

    return false;
}


void SATLAudioObjectData_FMOD::RemoveEvent(SATLEventData_FMOD *event)
{
    auto it = AZStd::find(m_events.begin(), m_events.end(), event);
    if(it != m_events.end())
    {
        m_events.erase(it);
    }
}


void SATLAudioObjectData_FMOD::StopEvent(uint32_t eventId)
{
    for(auto const event : m_events)
    {
        if(event->m_eventId == eventId)
        {
            event->m_evtInstance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
        }
    }
}


void SATLAudioObjectData_FMOD::StopAllEvents()
{
    for(auto const event : m_events)
    {
        event->m_evtInstance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
    }
}


void SATLAudioObjectData_FMOD::ApplyParameter(SATLParameterRtpc_FMOD *param, float value)
{
    for(auto event : m_events)
    {
        if(event->m_eventPathId != param->m_eventPathId)
            continue;

        auto it = g_ParameterToID.find(param);
        if(it != g_ParameterToID.end())
        {
            if(it->second.valid)
            {
                event->m_evtInstance->setParameterByID(it->second.id, param->m_mult * value + param->m_shift);
            }
            else
            {
                int paramCount = 0;
                event->m_description->getParameterDescriptionCount(&paramCount);

                for(int i = 0; i < paramCount; ++i)
                {
                    FMOD_STUDIO_PARAMETER_DESCRIPTION paramDesc;
                    event->m_description->getParameterDescriptionByIndex(i, &paramDesc);

                    if(AZ::StringFunc::Equal(param->m_name, paramDesc.name))
                    {
                        it->second = ParamId { true, paramDesc.id };
                        event->m_evtInstance->setParameterByID(paramDesc.id, param->m_mult * value + param->m_shift);
                        break;
                    }
                }

            }
        }
    }

    auto match = m_params.find(param);
    if(match != m_params.end() )
    {
        match->second = value;
    }
    else
    {
        m_params.emplace(param, value);
    }
}


void SATLAudioObjectData_FMOD::RemoveParameter(SATLParameterRtpc_FMOD *param)
{
    m_params.erase(param);
}


void SATLAudioObjectData_FMOD::ApplySwitch(SATLSwitchStateImplData_FMOD *switchImpl)
{
    for(auto event : m_events)
    {
        if(event->m_eventId == switchImpl->eventPathId)
        {
            auto it = g_SwitchToID.find(switchImpl);
            if(it != g_SwitchToID.end())
            {
                if(it->second.valid)
                {
                    event->m_evtInstance->setParameterByID(it->second.id, switchImpl->value);
                }
                else
                {
                    int paramCount = 0;
                    event->m_description->getParameterDescriptionCount(&paramCount);

                    for(int i = 0; i < paramCount; ++i)
                    {
                        FMOD_STUDIO_PARAMETER_DESCRIPTION paramDesc;
                        event->m_description->getParameterDescriptionByIndex(i, &paramDesc);
                        if(AZ::StringFunc::Equal(switchImpl->m_name, paramDesc.name))
                        {
                            it->second =  ParamId { true, paramDesc.id };
                            event->m_evtInstance->setParameterByID(paramDesc.id, switchImpl->value);
                            break;
                        }
                    }
                }
            }
        }
    }

    auto it = m_switches.find(switchImpl->eventPathId);
    if(it != m_switches.end())
    {
        it->second = switchImpl;
    }
    else
    {
        m_switches.emplace(switchImpl->eventPathId, switchImpl);
    }
}


void SATLAudioObjectData_FMOD::RemoveSwitch(SATLSwitchStateImplData_FMOD *switchImpl)
{
    m_switches.erase(switchImpl->eventPathId);
}


void SATLAudioObjectData_FMOD::ApplyEnvironment(SATLEnvironmentImplData_FMOD *env, float value)
{
    bool isSet = true;
    auto it = m_environments.find(env);
    if(it != m_environments.end())
    {
        isSet = AZ::Abs(it->second - value) > 0.001f;
        if(isSet)
        {
            it->second = value;
        }
    }
    else
    {
        m_environments.emplace(env, value);
    }

    if(isSet)
    {
        for(auto event : m_events)
        {
            event->TrySetEnvironment(env, value);
        }
    }
}


void SATLAudioObjectData_FMOD::RemoveEnvironment(SATLEnvironmentImplData_FMOD *env)
{
    m_environments.erase(env);
}

bool SATLEventData_FMOD::PrepareForOcclusion()
{
    m_masterTrack = nullptr;
    m_evtInstance->getChannelGroup(&m_masterTrack);

    if(m_masterTrack != nullptr)
    {
        if(!m_occlusionParamFound)
        {
            m_lpFilter = nullptr;
            int DSPcount = 0;

            m_masterTrack->getNumDSPs(&DSPcount);
            for(int i = 0; i < DSPcount; ++i)
            {
                FMOD_RESULT res = m_masterTrack->getDSP(i, &m_lpFilter);
                AZ_Assert(res == FMOD_OK, "Failure obtaining DSP at index %d", i);

                if(m_lpFilter != nullptr)
                {
                    FMOD_DSP_TYPE dspType;
                    m_lpFilter->getType(&dspType);

                    if(dspType == FMOD_DSP_TYPE_LOWPASS || dspType == FMOD_DSP_TYPE_LOWPASS_SIMPLE)
                    {
                        FMOD_DSP_PARAMETER_DESC* paramDesc = nullptr;
                        FMOD_RESULT res = m_lpFilter->getParameterInfo(FMOD_DSP_LOWPASS_CUTOFF, &paramDesc);
                        AZ_Assert(res == FMOD_OK, "Failure obtaining parameter description.");

                        m_lpfMin = paramDesc->floatdesc.min;
                        m_lpfMax = paramDesc->floatdesc.max;
                        break;
                    }
                    else
                    {
                        m_lpFilter = nullptr;
                    }
                }
            }
        }
    }

    return m_masterTrack != nullptr;
}

void SATLEventData_FMOD::SetObstructionOcclusion(float obstruction, float occlusion)
{
    AZ_UNUSED(obstruction);
    if(m_occlusionParamFound)
    {
        m_evtInstance->setParameterByID(m_occlusionParamID, occlusion);
    }
    else if(m_lpFilter != nullptr)
    {
        const float range = m_lpfMax - AZStd::max<float>(m_lpfMin, 10.0f);
        const float value = m_lpfMax - (occlusion * range);
        m_lpFilter->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, value);
    }
}

void SATLEventData_FMOD::TrySetEnvironment(SATLEnvironmentImplData_FMOD *envData, float value)
{
    if(!m_evtInstance && !m_masterTrack)
        return;

    FMOD::ChannelGroup* chnGroup = nullptr;
    envData->m_bus->getChannelGroup(&chnGroup);

    if(!chnGroup)
        return;

    FMOD::DSP* dspTail = nullptr;
    FMOD_RESULT res = chnGroup->getDSP(FMOD_CHANNELCONTROL_DSP_TAIL, &dspTail);
    AZ_Assert(res == FMOD_OK, "Failure retrieving DSP Tail from Channel Control!");

    if(dspTail)
    {
        int retId1 = (-1);
        dspTail->getParameterInt(FMOD_DSP_RETURN_ID, &retId1, nullptr, 0);

        int numDSPs = 0;
        m_masterTrack->getNumDSPs(&numDSPs);

        for(int i = 0; i < numDSPs; ++i)
        {
            FMOD::DSP* dspSend = nullptr;
            m_masterTrack->getDSP(i, &dspSend);

            if(dspSend != nullptr)
            {
                FMOD_DSP_TYPE dspType;
                dspSend->getType(&dspType);

                if(dspType == FMOD_DSP_TYPE_SEND)
                {
                    int retId2 = -1;
                    dspSend->getParameterInt(FMOD_DSP_RETURN_ID, &retId2, nullptr, 0);

                    if(retId1 == retId2)
                    {
                        dspSend->setParameterFloat(FMOD_DSP_SEND_LEVEL, value);
                        break;
                    }
                }
            }
        }
    }
}

void SATLEventData_FMOD::Reset()
{
    if(m_evtInstance)
    {
        m_evtInstance->release();
        m_evtInstance = nullptr;
    }

    if(m_audioObject)
    {
        m_audioObject->RemoveEvent(this);
        m_audioObject = nullptr;
    }

    m_eventPathId = 0;
    m_masterTrack = nullptr;
    m_description = nullptr;
}

}