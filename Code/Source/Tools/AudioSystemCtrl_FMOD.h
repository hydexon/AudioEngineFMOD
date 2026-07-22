#pragma once

#include <IAudioInterfacesCommonData.h>
#include <IAudioSystemControl.h>

namespace AudioEngineFMOD
{

    class IAudioSystemCtrl_FMOD : public AudioControls::IAudioSystemControl
    {
    public:
        IAudioSystemCtrl_FMOD() {};
        IAudioSystemCtrl_FMOD(const AZStd::string& name, AudioControls::CID id, AudioControls::TImplControlType type);
        ~IAudioSystemCtrl_FMOD() override {};
    };

}