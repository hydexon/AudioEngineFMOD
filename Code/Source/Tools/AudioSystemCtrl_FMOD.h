#pragma once

#include <IAudioInterfacesCommonData.h>
#include <IAudioSystemControl.h>

namespace AudioEngineFMOD
{
    enum EFMODStudioControlTypes
    {
        eFMOD_INVALID = 0,
        eFMOD_FOLDER = AUDIO_BIT(1),
        eFMOD_EVENT = AUDIO_BIT(2),
        eFMOD_EVENT_PARAMETER = AUDIO_BIT(3),
        eFMOD_SNAPSHOT = AUDIO_BIT(4),
        eFMOD_SNAPSHOT_PARAMETER = AUDIO_BIT(5),
        eFMOD_SOUNDBANK = AUDIO_BIT(6),
        eFMOD_RETURN = AUDIO_BIT(7),
        eFMOD_GROUP = AUDIO_BIT(8)
    };

    class IAudioSystemCtrl_FMOD : public AudioControls::IAudioSystemControl
    {
    public:
        IAudioSystemCtrl_FMOD() {};
        IAudioSystemCtrl_FMOD(const AZStd::string& name, AudioControls::CID id, AudioControls::TImplControlType type);
        ~IAudioSystemCtrl_FMOD() override {};
    };

}