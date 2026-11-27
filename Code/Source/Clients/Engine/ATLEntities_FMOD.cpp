#include "ATLEntities_FMOD.h"

//#include "Common_FMOD.h"
#include "AudioSystemImplCVars.h"
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
            m_audioEvents.push_back(event);
            for (const auto& paramPair : m_parameters)
            {
                if(event->GetEventPathId() == paramPair.first->m_targetId)
                {
                    event->GetInstance()->setParameterByID(paramPair.first->m_paramId, paramPair.second);
                }
            }

            /*
            //Switches.
            for (const auto& swPair : m_switches)
            {
                auto const it = g_SwitchToID.find(swPair.second);
                if (it != g_SwitchToID.end())
                {
                    if (event->GetEventPathId() == it->first->m_eventPathId)
                    {
                        if (it->second.valid)
                        {
                            event->GetInstance()->setParameterByID(it->second.id, swPair.second->m_value);
                        }
                        else
                        {
                            SetSwitch(swPair.second);
                        }
                    }
                }
            }
            //Environments:
            for (const auto& enviroPair : m_environments)
            {
                event->TrySetEnvironment(enviroPair.first, enviroPair.second);
            }*/

            event->GetInstance()->start();
            suceess = true;
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

    void ATLAudioObject_FMOD::PostParameter(const ATLParameterRtpc_FMOD *rtpc, const float value)
    {
        for (const auto evt : m_audioEvents)
        {
            if (evt->GetEventPathId() == rtpc->m_targetId)
            {
                evt->GetInstance()->setParameterByID(rtpc->m_paramId, rtpc->m_mult * value + rtpc->m_shift);
            }
        }

        const auto iter = m_parameters.find(rtpc);
        if (iter != m_parameters.end())
        {
            iter->second = value;
        }
        else
        {
            m_parameters.emplace(rtpc, value);
        }
    }

    void ATLAudioObject_FMOD::ResetParameter(const ATLParameterRtpc_FMOD *rtpc)
    {
        for(const auto evt : m_audioEvents)
        {
            if(evt->GetEventPathId() == rtpc->m_targetId)
            {
                evt->GetInstance()->setParameterByID(rtpc->m_paramId, rtpc->m_defaultValue);
            }
        }
    }

    void ATLAudioObject_FMOD::UnpostParameter(ATLParameterRtpc_FMOD* rtpc)
    {
        m_parameters.erase(rtpc);
    }

    void ATLAudioObject_FMOD::SetSwitch(ATLSwitchState_FMOD* state)
    {
        for (const auto evt : m_audioEvents)
        {
            if (evt->GetEventPathId() == state->m_eventPathId)
            {

            }
        }

        const auto iter = m_switches.find(state->m_eventPathId);
        if (iter != m_switches.end())
        {
            iter->second = state;
        }
        else
        {
            m_switches.emplace(state->m_eventPathId, state);
        }
    }

    void ATLAudioObject_FMOD::UnsetSwitch(ATLSwitchState_FMOD* state)
    {
        m_switches.erase(state->m_eventPathId);
    }

    void ATLAudioObject_FMOD::SetEnvironment(ATLEnvironment_FMOD* env, const float value)
    {
    }

    void ATLAudioObject_FMOD::UnsetEnvironment(ATLEnvironment_FMOD* env)
    {
        m_environments.erase(env);
    }

    void ATLAudioObject_FMOD::Set3DAttributes(const Audio::SATLWorldPosition &pos)
    {
        m_attributes = Utils::CreateFMOD3DAttributes(pos.GetPositionVec(), pos.GetForwardVec(), pos.GetUpVec());
        for (const auto evt : m_audioEvents)
        {
            evt->GetInstance()->set3DAttributes(&m_attributes);
        }
    }

    void ATLAudioObject_FMOD::StopAllEvents()
    {
        for (const auto evt : m_audioEvents)
        {
            evt->GetInstance()->stop(FMOD_STUDIO_STOP_IMMEDIATE);
        }
    }

    void ATLAudioObject_FMOD::StopPlayingEvent(uint32_t eventPathId)
    {
        for (const auto evt : m_audioEvents)
        {
            if (evt->GetEventPathId() == eventPathId)
            {
                evt->GetInstance()->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
            }
        }
    }

    void ATLAudioObject_FMOD::SetObstructionOcclusion(float obstruction, float occlusion)
    {
        for (const auto evt : m_audioEvents)
        {
            evt->SetOcclusionObstruction(occlusion, obstruction);
        }
    }

    void ATLAudioObject_FMOD::Reset()
    {
        m_audioEvents.clear();
        m_parameters.clear();
        m_switches.clear();
    }

    void ATLEventData_FMOD::Reset()
    {
        if (m_instance != nullptr)
        {
            m_instance->release();
            m_instance = nullptr;
        }

        if (m_audioObject != nullptr)
        {
            m_audioObject->UnpostAudioEvent(this);
            m_audioObject = nullptr;
        }

        m_eventPathId = INVALID_AUDIO_EVENT_ID;
        m_masterTrack = nullptr;
    }

    bool ATLEventData_FMOD::PrepareForOcclusion()
    {
        m_masterTrack = nullptr;
        FMOD_RESULT result = m_instance->getChannelGroup(&m_masterTrack);

        if (m_masterTrack != nullptr && result == FMOD_OK)
        {
            if (!m_occlusionParamFound)
            {
                int numDSPs = 0;
                m_masterTrack->getNumDSPs(&numDSPs);

                for (int i = 0; i < numDSPs; ++i)
                {
                    m_masterTrack->getDSP(i, &m_lpDsp);
                    if (m_lpDsp != nullptr)
                    {
                        FMOD_DSP_TYPE dspType;
                        m_lpDsp->getType(&dspType);

                        if (dspType == FMOD_DSP_TYPE_LOWPASS || dspType == FMOD_DSP_TYPE_LOWPASS_SIMPLE)
                        {
                            FMOD_DSP_PARAMETER_DESC* dspParamDesc = nullptr;
                            m_lpDsp->getParameterInfo(FMOD_DSP_LOWPASS_CUTOFF, &dspParamDesc);
                            m_lpCutoffMax = dspParamDesc->floatdesc.max;
                            m_lpCutoffMIn = dspParamDesc->floatdesc.min;
                            break;
                        }
                        else
                        {
                            m_lpDsp = nullptr;
                        }
                    }
                }
            }
        }
        return m_masterTrack != nullptr;
    }

    void ATLEventData_FMOD::SetOcclusionParameterID(FMOD_STUDIO_PARAMETER_ID& paramId)
    {
        m_occlusionParameterId = paramId;
        m_occlusionParamFound = true;
    }

    void ATLEventData_FMOD::SetOcclusionObstruction(float occlusion, [[maybe_unused]] float obstruction)
    {
        if (m_occlusionParamFound)
        {
            m_instance->setParameterByID(m_occlusionParameterId, occlusion);
        }
        else if (m_lpDsp)
        {
            const float range = m_lpCutoffMax - std::max(m_lpCutoffMIn, (float)CVars::s_FMODStudio_DSPLpfMinCutoffFreq);
            const float value = m_lpCutoffMax - (occlusion * range);
            m_lpDsp->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, value);
        }
    }

    void ATLEventData_FMOD::TrySetEnvironment(ATLEnvironment_FMOD* environment, float value) const
    {
        if (m_instance == nullptr && m_masterTrack == nullptr)
        {
            return;
        }

        FMOD::ChannelGroup* chnGroup = nullptr;
        environment->m_bus->getChannelGroup(&chnGroup);
        if (chnGroup == nullptr)
            return;

        FMOD::DSP* dspReturn = nullptr;
        chnGroup->getDSP(FMOD_CHANNELCONTROL_DSP_TAIL, &dspReturn);
        if (dspReturn != nullptr)
        {
            int returnId1 = -1;
            dspReturn->getParameterInt(FMOD_DSP_RETURN_ID, &returnId1, nullptr, 0);

            int numDSPs =0;
            m_masterTrack->getNumDSPs(&numDSPs);
            for (int i = 0; i < numDSPs; ++i)
            {
                FMOD::DSP* dspSend = nullptr;
                m_masterTrack->getDSP(i, &dspSend);

                if (dspSend != nullptr)
                {
                    FMOD_DSP_TYPE dspType;
                    dspSend->getType(&dspType);
                    if (dspType == FMOD_DSP_TYPE_SEND)
                    {
                        int returnId2 = -1;
                        dspSend->getParameterInt(FMOD_DSP_RETURN_ID, &returnId2, nullptr, 0);
                        if (returnId1 == returnId2)
                        {
                            dspSend->setParameterFloat(FMOD_DSP_SEND_LEVEL, value);
                            break;
                        }
                    }
                }
            }
        }
    }
}
