#pragma once

#include <ATLEntityData.h>
#include <fmod_studio.hpp>
#include "Common_FMOD.h"

#include <AzCore/XML/rapidxml.h>

namespace AudioEngineFMOD
{
    typedef AZStd::fixed_string<256> FixedEventPath;
    struct SATLAudioObjectData_FMOD : public Audio::IATLAudioObjectData
    {
        FMOD_3D_ATTRIBUTES m_3dAttributes = {};
        AZStd::vector<FMOD::Studio::EventInstance*> m_activeInstances;
        //AZStd::unordered_multimap<FMOD_GUID, FMOD::Studio::EventInstance*, FMOD_GUID_CmpEqual, FMOD_GUID_Hashing> m_instances;
    };

    enum class RtpcImpl {
        SingleEvent,
        PerObject,
        Global,
        Unknown
    };

    struct SATLAudioRtpcImplData_FMOD : public Audio::IATLRtpcImplData
    {
        RtpcImpl m_type;
        FMOD_GUID m_paramGUID;
        AZStd::string m_paramPath;
        FixedEventPath m_singleEvtPath;

        bool ReadFromXml(const AZ::rapidxml::xml_node<char>& node);
    };

    struct SATLEventData_FMOD : public Audio::IATLEventData
    {
        explicit SATLEventData_FMOD(const Audio::TAudioControlID AudioEvtId)
            : m_eventId(AudioEvtId)
        {};

        Audio::TAudioEventID m_eventId;
        AZStd::string atlName;
        FixedEventPath m_eventPath;
        FMOD::Studio::EventDescription* m_eventDescription = nullptr;
        FMOD::Studio::EventInstance* m_currentInstance = nullptr;
        FMOD_STUDIO_STOP_MODE m_stopMode = FMOD_STUDIO_STOP_ALLOWFADEOUT;
        FMODEventAction m_actionMode = FMODEventAction::Play;
    };


    struct SATLTriggerImplData_FMOD : public Audio::IATLTriggerImplData
    {
        AZStd::string atlName;
        FixedEventPath m_eventPath;
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