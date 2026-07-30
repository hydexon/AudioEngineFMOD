#pragma once

#include <IAudioConnection.h>

namespace AudioEngineFMOD
{
    class CFMODBankConnection : public AudioControls::IAudioConnection
    {
    public:
        explicit CFMODBankConnection(AudioControls::CID id)
            : AudioControls::IAudioConnection(id)
            , loadSampleData(false)
        {
        }

        ~CFMODBankConnection() = default;

        bool HasProperties() override { return true; }
        bool loadSampleData;
    };

    //TODO: Snapshot Connection?

    class CFMODEventConnection : public AudioControls::IAudioConnection
    {
    public:
        explicit CFMODEventConnection(AudioControls::CID id)
            : AudioControls::IAudioConnection(id)
        {
        }

        ~CFMODEventConnection() = default;
        bool HasProperties() override { return true; }

        bool loadSampleData;
        //TODO: Enum here to Play or Stop Event.
        //      If Stop Event also show how it should stop.
    };
}