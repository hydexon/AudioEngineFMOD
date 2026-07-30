#pragma once

#include <ATLEntityData.h>
#include <fmod_studio.hpp>

namespace AudioEngineFMOD
{
    struct SATLAudioObjectData_FMOD : public Audio::IATLAudioObjectData
    {
        FMOD_3D_ATTRIBUTES m_3dAttributes = {};
        AZStd::vector<FMOD::Studio::EventInstance*> m_activeInstances;
    };

    struct SATLEventData_FMOD : public Audio::IATLEventData
    {
    };


    struct SATLTriggerImplData_FMOD : public Audio::IATLTriggerImplData
    {
        FMOD::Studio::EventDescription* m_eventDescription;
        bool m_preloadSampleData = false;
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
            : pFMODBank(nullptr)
            , m_baseBankName(bankBaseName)
            , m_loadSampleData(loadSampleData)
        {}

        ~SATLAudioFileEntryData_FMOD() override {}

        bool m_loadSampleData;
        AZStd::string m_baseBankName;
        FMOD::Studio::Bank* pFMODBank;
    };

}