#pragma once

#include <IAudioInterfacesCommonData.h>
#include <IAudioSystemControl.h>

namespace AudioEngineFMOD
{
    enum EFMODStudioControlTypes
    {
        eFMOD_INVALID = 0,
        eFMOD_EVENT = AUDIO_BIT(0),
        eFMOD_PARAMETER = AUDIO_BIT(1),
        eFMOD_SOUNDBANK = AUDIO_BIT(2),
        eFMOD_AUXBUS    = AUDIO_BIT(3),
        eFMOD_LABELEDPARAM = AUDIO_BIT(4),
        eFMOD_SNAPSHOT     = AUDIO_BIT(5)
    };

    class IAudioSystemCtrl_FMOD : public AudioControls::IAudioSystemControl
    {
    public:
        IAudioSystemCtrl_FMOD() {};
        IAudioSystemCtrl_FMOD(const AZStd::string& name, AudioControls::CID id, AudioControls::TImplControlType type);
        ~IAudioSystemCtrl_FMOD() override {};
    };

}