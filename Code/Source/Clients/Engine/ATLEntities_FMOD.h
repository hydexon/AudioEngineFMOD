#pragma once

#include <ATLEntityData.h>
#include <fmod_studio.hpp>
#include "Common_FMOD.h"

#include <AzCore/XML/rapidxml.h>

namespace AudioEngineFMOD
{

    struct SATLAudioObjectData_FMOD : public Audio::IATLAudioObjectData
    {
        explicit SATLAudioObjectData_FMOD(Audio::TAudioControlID id);

        Audio::TAudioControlID m_id;
        FMOD_3D_ATTRIBUTES m_attributes;

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

    struct SATLEventData_FMOD : public Audio::IATLEventData
    {
        explicit SATLEventData_FMOD(const Audio::TAudioControlID AudioEvtId)
            : m_eventId(AudioEvtId)
        {};

        Audio::TAudioEventID m_eventId;
    };

    struct SATLTriggerImplData_FMOD : public Audio::IATLTriggerImplData
    {
    };

    struct SATLEnvironmentImplData_FMOD : public Audio::IATLEnvironmentImplData
    {
    };

    struct SATLSwitchStateImplData_FMOD : public Audio::IATLSwitchStateImplData
    {
    };

    struct SATLListenerData_FMOD : public Audio::IATLListenerData
    {
        explicit SATLListenerData_FMOD(const Audio::TAudioControlID id)
            : m_id(id)
        {
        }

        Audio::TAudioControlID m_id;
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