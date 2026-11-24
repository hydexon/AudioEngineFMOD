#pragma once

#include "Common_FMOD.h"
#include <ATLEntityData.h>
#include <fmod_studio.hpp>
#include <AudioAllocators.h>

namespace AudioEngineFMOD
{
    struct SATLEventData_FMOD;
    struct SATLParameterRtpc_FMOD;
    struct SATLSwitchStateImplData_FMOD;
    struct SATLEnvironmentImplData_FMOD;

    struct SATLAudioObjectData_FMOD : public Audio::IATLAudioObjectData
    {
    };

    struct SATLEventData_FMOD : public Audio::IATLEventData
    {
    };

    struct SATLParameterRtpc_FMOD : public Audio::IATLRtpcImplData
    {
    };

    struct SATLTriggerImplData_FMOD : public Audio::IATLTriggerImplData
    {
        explicit SATLTriggerImplData_FMOD(
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

    struct SATLEnvironmentImplData_FMOD : public Audio::IATLEnvironmentImplData
    {
    };

    struct SATLSwitchStateImplData_FMOD : public Audio::IATLSwitchStateImplData
    {
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