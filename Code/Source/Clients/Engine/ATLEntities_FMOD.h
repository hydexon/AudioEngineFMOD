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
        void PostParameter(const ATLParameterRtpc_FMOD* rtpc, const float value);
        void ResetParameter(const ATLParameterRtpc_FMOD* rtpc);
        void UnpostParameter(ATLParameterRtpc_FMOD* rtpc);
        void SetSwitch(const ATLSwitchState_FMOD* state);
        void UnsetSwitch(const ATLSwitchState_FMOD* state);
        void SetEnvironment(const ATLEnvironment_FMOD* env, const float amount);
        void UnsetEnvironment(const ATLEnvironment_FMOD* env);
        void Set3DAttributes(const Audio::SATLWorldPosition& pos);
        void StopAllEvents();
        void StopPlayingEvent(uint32_t eventPathId);
        void SetObstructionOcclusion(float obstruction, float occlusion);
        void Reset();

    private:
        Audio::TAudioObjectID m_id;
        FMOD_3D_ATTRIBUTES m_attributes;
        AZStd::vector<ATLEventData_FMOD*> m_audioEvents;
        AZStd::unordered_map<const ATLParameterRtpc_FMOD*, float> m_parameters;
        AZStd::unordered_map<uint32_t, const ATLSwitchState_FMOD*> m_switches;
        AZStd::unordered_map<const ATLEnvironment_FMOD*, float> m_environments;
    };

    //NOTE: They should have an EventDescription instance, to get the parameters-needed information?
    struct ATLEventData_FMOD : public Audio::IATLEventData
    {
        explicit ATLEventData_FMOD(Audio::TAudioEventID id)
            : m_id(id)
            , m_eventPathId(INVALID_AUDIO_EVENT_ID)
            , m_lpCutoffMax(0.0f)
            , m_lpCutoffMIn(0.0f)
            , m_instance(nullptr)
            , m_masterTrack(nullptr)
            , m_lpDsp(nullptr)
            , m_occlusionParamFound(false)
            , m_occlusionParameterId({})
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
        void SetOcclusionParameterID(FMOD_STUDIO_PARAMETER_ID& paramId);
        void SetOcclusionObstruction(float occlusion, float obstruction);
        void TrySetEnvironment(const ATLEnvironment_FMOD* environment, float value) const;
    private:
        Audio::TAudioEventID m_id;
        AZ::u32 m_eventPathId;

        FMOD::Studio::EventInstance* m_instance;
        FMOD::ChannelGroup* m_masterTrack;
        FMOD::DSP* m_lpDsp;
        FMOD_STUDIO_PARAMETER_ID m_occlusionParameterId;

        bool m_occlusionParamFound;
        float m_lpCutoffMax;
        float m_lpCutoffMIn;

        ATLAudioObject_FMOD* m_audioObject;
    };

    struct ATLParameterRtpc_FMOD : public Audio::IATLRtpcImplData
    {
        explicit  ATLParameterRtpc_FMOD()
            : m_shift(0.0f)
            , m_mult(1.0f)
            , m_isGlobalParameter(false)
            , m_paramId({0})
            , m_targetId(INVALID_AUDIO_EVENT_ID)
            , m_validParamId(false)
            , m_defaultValue(0.0f)
        {}
        ~ATLParameterRtpc_FMOD() override = default;

        AZStd::fixed_string<256> m_targetPath;
        AZStd::fixed_string<256> m_paramName;
        mutable FMOD_STUDIO_PARAMETER_ID m_paramId;
        AZ::u32 m_targetId;
        bool m_isGlobalParameter;
        float m_mult;
        float m_shift;
        mutable bool m_validParamId;
        mutable float m_defaultValue;
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
        explicit ATLSwitchState_FMOD()
        : m_value(0.0f)
        , m_isGlobal(false)
        , m_targetEventPathId(INVALID_AUDIO_CONTROL_ID)
        , m_paramIdValid(false)
        , m_paramId({0})
        {}

        uint32_t m_targetEventPathId;
        AZStd::fixed_string<256> m_targetPath;
        AZStd::fixed_string<128> m_name;
        mutable FMOD_STUDIO_PARAMETER_ID m_paramId;
        float m_value;
        bool m_isGlobal;
        mutable bool m_paramIdValid;
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