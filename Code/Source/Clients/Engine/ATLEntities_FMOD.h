#pragma once

#include <ATLEntityData.h>
#include <fmod_studio.hpp>
#include "Common_FMOD.h"
#include <AudioAllocators.h>
#include <AzCore/XML/rapidxml.h>

namespace AudioEngineFMOD
{
    struct SATLEventData_FMOD;
    struct SATLParameterRtpc_FMOD;
    struct SATLSwitchStateImplData_FMOD;
    struct SATLEnvironmentImplData_FMOD;

    struct SATLAudioObjectData_FMOD : public Audio::IATLAudioObjectData
    {
        explicit SATLAudioObjectData_FMOD(Audio::TAudioControlID id);
        virtual ~SATLAudioObjectData_FMOD() {}

        void Reset();
        bool ApplyEvent(SATLEventData_FMOD* event);
        void RemoveEvent(SATLEventData_FMOD* event);
        void StopEvent(uint32_t eventId);
        void StopAllEvents();

        void ApplyParameter(SATLParameterRtpc_FMOD* param);
        void RemoveParameter(SATLParameterRtpc_FMOD* param);

        void ApplySwitch(SATLSwitchStateImplData_FMOD* switchImpl);
        void RemoveSwitch(SATLSwitchStateImplData_FMOD* switchImpl);

        void ApplyEnvironment(SATLEnvironmentImplData_FMOD* env);
        void RemoveEnvironment(SATLEnvironmentImplData_FMOD* env);

        Audio::TAudioControlID m_id;
        FMOD_3D_ATTRIBUTES m_attributes;
        AZStd::fixed_string<512> m_name;

        AZStd::vector<SATLEventData_FMOD*> m_events;
        AZStd::unordered_map<SATLParameterRtpc_FMOD*, float> m_params;
        AZStd::unordered_map<AZ::u32, SATLSwitchStateImplData_FMOD*> m_switches;
        AZStd::unordered_map<SATLEnvironmentImplData_FMOD*, float> m_environments;
    };

    struct SATLEventData_FMOD : public Audio::IATLEventData
    {
        explicit SATLEventData_FMOD(const Audio::TAudioControlID AudioEvtId)
            : m_eventId(AudioEvtId)
        {};

        bool CheckOcclusionEnabled();
        void SetObstructionOcclusion(float obstruction, float occlusion);
        void TrySetEnvironment(SATLEnvironmentImplData_FMOD* envData);
        void Reset();

        Audio::TAudioEventID m_eventId;
        uint32_t m_eventPathId;
        float m_lpfMax = 0.0f;
        float m_lpfMin = 0.0f;

        FMOD::Studio::EventInstance* m_evtInstance;
        FMOD::ChannelGroup* m_masterTrack;
        FMOD::DSP* m_lp;
        FMOD_STUDIO_PARAMETER_ID m_occlusionParamID;

        SATLAudioObjectData_FMOD* m_audioObject;
    };

    struct SATLParameterRtpc_FMOD : public Audio::IATLRtpcImplData
    {
        explicit SATLParameterRtpc_FMOD(
                uint32_t eventPathId,
                float const mult,
                float const shift)
            : m_eventPathId(eventPathId)
            , m_mult(mult)
            , m_shift(shift)
        {};

        uint32_t m_eventPathId;
        float const m_mult;
        float const m_shift;
    };

    struct SATLTriggerImplData_FMOD : public Audio::IATLTriggerImplData
    {
        uint32_t m_eventPathId;
        FMODEventType m_eventAction;
        FMOD::Studio::EventDescription* m_eventDescription;
        FMOD_GUID m_guid;
    };

    struct SATLEnvironmentImplData_FMOD : public Audio::IATLEnvironmentImplData
    {
        FMOD::Studio::EventDescription* m_eventDescription;
        FMOD::Studio::Bus* m_bus;
    };

    struct SATLSwitchStateImplData_FMOD : public Audio::IATLSwitchStateImplData
    {
        uint32_t eventPathId;
        float value;
        AZStd::fixed_string<512> m_name;
    };

    struct SATLListenerData_FMOD : public Audio::IATLListenerData
    {
        explicit SATLListenerData_FMOD(const int id)
            : m_id(id)
        {
        }

        int m_id;
        FMOD_3D_ATTRIBUTES m_attributes;
    };

    struct SATLAudioFileEntryData_FMOD : public Audio::IATLAudioFileEntryData
    {
        SATLAudioFileEntryData_FMOD(const char* bankBaseName)
            : m_FMODBank(nullptr)
            , m_baseBankName(bankBaseName)
        {}

        ~SATLAudioFileEntryData_FMOD() override {}

        AZStd::string m_baseBankName;
        FMOD::Studio::Bank* m_FMODBank;
    };

}