#pragma once

#include <ATLEntityData.h>
#include <fmod_studio.hpp>
#include "Common_FMOD.h"

namespace AudioEngineFMOD
{

    struct SATLAudioObjectData_FMOD : public Audio::IATLAudioObjectData
    {
        FMOD_3D_ATTRIBUTES m_3dAttributes = {};
        AZStd::vector<FMOD::Studio::EventInstance*> m_activeInstances;
    };

    struct SATLEventData_FMOD : public Audio::IATLEventData
    {
        AZStd::fixed_string<256> m_eventPath;
        FMOD::Studio::EventDescription* m_eventDescription = nullptr;
        FMOD::Studio::EventInstance* m_currentInstance = nullptr;
        FMOD_STUDIO_STOP_MODE m_stopMode = FMOD_STUDIO_STOP_ALLOWFADEOUT;
        FMODEventAction m_actionMode = FMODEventAction::Play;
    };


    struct SATLTriggerImplData_FMOD : public Audio::IATLTriggerImplData
    {
        AZStd::fixed_string<256> m_eventPath;
        FMOD_GUID m_eventGUID;
        bool m_preloadSampleData = false;
        FMOD_STUDIO_STOP_MODE m_stopMode = FMOD_STUDIO_STOP_ALLOWFADEOUT;
        FMODEventAction m_action = FMODEventAction::Unknown;
    };

    struct SATLListenerData_FMOD : public Audio::IATLListenerData
    {
        SATLListenerData_FMOD(int lidx = 0)
            : listenerIndex(lidx)
        {}

        int listenerIndex = 0;
        AZ::Vector3 velocity = AZ::Vector3::CreateOne();
        float weight = 1.0f;
    };

    struct SATLAudioFileEntryData_FMOD : public Audio::IATLAudioFileEntryData
    {
        SATLAudioFileEntryData_FMOD(const char* bankBaseName, bool loadSampleData)
            : m_FMODBank(nullptr)
            , m_baseBankName(bankBaseName)
            , m_loadSampleData(loadSampleData)
        {}

        ~SATLAudioFileEntryData_FMOD() override {}

        bool m_loadSampleData;
        AZStd::string m_baseBankName;
        FMOD::Studio::Bank* m_FMODBank;
    };

}