#pragma once

#include <IAudioConnection.h>
#include "../../Clients/Engine/Common_FMOD.h"
#include <fmod_studio_common.h>

namespace AudioEngineFMOD
{
    class CFMODBankConnection : public AudioControls::IAudioConnection
    {
    public:
        explicit CFMODBankConnection(AudioControls::CID id)
            : AudioControls::IAudioConnection(id)
            , m_loadSampleData(false)
        {
        }

        ~CFMODBankConnection() = default;

        bool HasProperties() override { return true; }
        bool m_loadSampleData;
    };

    class CFMODEventParamRTPC : public AudioControls::IAudioConnection
    {
    public:
        explicit CFMODEventParamRTPC(AudioControls::CID id)
            : AudioControls::IAudioConnection(id)
        {}

        ~CFMODEventParamRTPC() = default;
        bool HasProperties() override { return true; }

        bool perObject = false;
        AZStd::string m_paramName = "";
    };

    //TODO: Snapshot Connection?

    class CFMODEventConnection : public AudioControls::IAudioConnection
    {
    public:
        explicit CFMODEventConnection(AudioControls::CID id)
            : AudioControls::IAudioConnection(id)
            , m_loadSampleData(false)
            , m_stopMode(FMOD_STUDIO_STOP_ALLOWFADEOUT)
            , m_action(FMODEventAction::Play)
        {
        }

        ~CFMODEventConnection() = default;
        bool HasProperties() override { return true; }

        bool m_isSnapshot;
        FMODEventAction m_action;
        bool m_loadSampleData;
        FMOD_STUDIO_STOP_MODE m_stopMode;

        //TODO: Enum here to Play or Stop Event.
        //      If Stop Event also show how it should stop.
    };
}