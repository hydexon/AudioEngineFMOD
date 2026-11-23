#pragma once

#include <IAudioConnection.h>
#include "Clients/Engine/Common_FMOD.h"
#include <fmod_studio_common.h>

namespace AudioEngineFMOD
{
    class FMODEventConnection : public AudioControls::IAudioConnection
    {
    public:
        explicit FMODEventConnection(AudioControls::CID id)
            : IAudioConnection(id)
            , m_eventType(FMODEventType::Play)
        {}

        bool HasProperties() override
        {
            return true;
        }

        FMODEventType m_eventType;
    };
    
    class FMODParamRtpcConnection : public AudioControls::IAudioConnection
    {
    public:
        explicit FMODParamRtpcConnection(AudioControls::CID id)
            : IAudioConnection(id)
            , m_mult(1.0f)
            , m_shift(0.0f)
        {}
        bool HasProperties() override
        {
            return true;
        }

        float m_mult;
        float m_shift;
    };

    class FMODParamToStateConnection : public AudioControls::IAudioConnection
    {
    public:
        explicit FMODParamToStateConnection(AudioControls::CID id)
            : IAudioConnection(id)
            , m_value(1.0f)
        {};

        bool HasProperties() override
        {
            return true;
        }

        float m_value;
    };


}