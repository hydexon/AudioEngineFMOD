#pragma once

#include "Common_FMOD.h"
#include <ATLEntityData.h>
#include <fmod_studio.hpp>
#include <AudioAllocators.h>

namespace AudioEngineFMOD
{
    struct ATLAudioObject_FMOD;
    struct ATLEventData_FMOD;
    struct ATLParameterRtpc_FMOD;
    struct ATLSwitchState_FMOD;
    struct ATLEnvironment_FMOD;

    typedef AZStd::vector<ATLAudioObject_FMOD> AudioObjectsVec;

    struct ATLAudioObject_FMOD : public Audio::IATLAudioObjectData
    {
        explicit ATLAudioObject_FMOD(Audio::TAudioObjectID id);
        ~ATLAudioObject_FMOD() override = default;

        Audio::TAudioObjectID GetId() const { return m_id; }
        FMOD_3D_ATTRIBUTES& Get3DAttributes() { return m_attributes; }
        bool PostAudioEvent(ATLEventData_FMOD* event);
        void UnpostAudioEvent(ATLEventData_FMOD* event);
        bool PostParameter(ATLParameterRtpc_FMOD* rtpc);
        void UnpostParameter(ATLParameterRtpc_FMOD* rtpc);
        void SetSwitch(ATLSwitchState_FMOD* state);
        void SetEnvironment(ATLEnvironment_FMOD* env);
        void Set3DAttributes(Audio::SATLWorldPosition& pos);
        void StopAllEvents();
        void StopPlayingEvent(uint32_t eventPathId);
        void SetObstructionOcclusion(float obstruction, float occlusion);
        void Reset();

    private:
        Audio::TAudioObjectID m_id;
        FMOD_3D_ATTRIBUTES m_attributes;
        AZStd::vector<ATLEventData_FMOD*> m_audioEvents;
        AZStd::unordered_map<ATLParameterRtpc_FMOD*, float> m_parameters;
        AZStd::unordered_map<uint32_t, ATLSwitchState_FMOD*> m_switches;
        AZStd::unordered_map<ATLEnvironment_FMOD*, float> m_environments;
    };

    struct ATLEventData_FMOD : public Audio::IATLEventData
    {
        explicit ATLEventData_FMOD(Audio::TAudioEventID id)
            : m_id(id)
            , m_eventPathId(INVALID_AUDIO_OBJECT_ID)
            , m_lpCutoffMax(0.0f)
            , m_lpCutoffMIn(0.0f)
            , m_instance(nullptr)
            , m_masterTrack(nullptr)
            , m_lpDsp(nullptr)
            , m_occlusionParameterDesc({})
            , m_audioObject(nullptr)
        {}
        ~ATLEventData_FMOD() override = default;

        void Reset();
        Audio::TAudioEventID GetId() const  { return m_id; }

        AZ::u32 GetEventPathId() const { return m_eventPathId; }
        void SetEventPathId(AZ::u32 id) { m_eventPathId = id; }
        ATLAudioObject_FMOD* GetATLAudioObject() const { return m_audioObject; }
        void SetATLAudioObject(ATLAudioObject_FMOD* obj) { m_audioObject = obj;}
        FMOD::Studio::EventInstance* GetInstance() const { return m_instance; }
        void SetInstance(FMOD::Studio::EventInstance* instance) { m_instance = instance; }

        bool PrepareForOcclusion();
        void SetOcclusionObstruction(float occlusion, float obstruction);
        void TrySetEnvironment(ATLEnvironment_FMOD* environment, float value);
    private:
        Audio::TAudioEventID m_id;
        AZ::u32 m_eventPathId;

        FMOD::Studio::EventInstance* m_instance;
        FMOD::ChannelGroup* m_masterTrack;
        FMOD::DSP* m_lpDsp;
        FMOD_STUDIO_PARAMETER_DESCRIPTION m_occlusionParameterDesc;

        float m_lpCutoffMax;
        float m_lpCutoffMIn;

        ATLAudioObject_FMOD* m_audioObject;
    };

    struct ATLParameterRtpc_FMOD : public Audio::IATLRtpcImplData
    {
        explicit  ATLParameterRtpc_FMOD(AZ::u32 pathId, float shift, float mult, bool isGlobal = false)
            : m_shift(shift)
            , m_mult(mult)
            , m_paramPathId(pathId)
            , m_isGlobalParameter(isGlobal)
        {}
        ~ATLParameterRtpc_FMOD() override = default;

        AZ::u32 m_paramPathId;
        bool m_isGlobalParameter;
        float m_mult;
        float m_shift;

    };

    struct ATLTrigger_FMOD : public Audio::IATLTriggerImplData
    {
        explicit ATLTrigger_FMOD(
                Audio::TAudioTriggerImplID id,
                FMODEventType evtType,
                FMOD::Studio::EventDescription* evtDesc,
                FMOD_GUID guid)
            : m_id(id)
            , m_type(evtType)
            , m_eventDescription(evtDesc)
            , m_guid(guid)
        {}

        Audio::TAudioTriggerImplID m_id;
        FMODEventType m_type;
        FMOD::Studio::EventDescription* m_eventDescription;
        FMOD_GUID m_guid;
    };

    struct ATLEnvironment_FMOD : public Audio::IATLEnvironmentImplData
    {
        explicit ATLEnvironment_FMOD(
            FMOD::Studio::EventDescription* evtDesc,
            FMOD::Studio::Bus* bus)
                : m_description(evtDesc)
                , m_bus(bus) {}

        FMOD::Studio::EventDescription* m_description;
        FMOD::Studio::Bus* m_bus;
    };

    struct ATLSwitchState_FMOD : public Audio::IATLSwitchStateImplData
    {
        explicit ATLSwitchState_FMOD(
            uint32_t evtPathId,
            float value,
            bool isGlobal,
            const char* name)
        : m_eventPathId(evtPathId)
        , m_value(value)
        , m_isGlobal(isGlobal)
        , m_name(name)
        {}

        uint32_t m_eventPathId;
        float m_value;
        bool m_isGlobal;
        AZStd::fixed_string<256> m_name;
    };

    struct ATLListenerData_FMOD : public Audio::IATLListenerData
    {
        explicit ATLListenerData_FMOD(const int id)
            : m_id(id)
            , m_attributes({})
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