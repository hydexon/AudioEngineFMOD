#include "AudioSystemCtrl_FMOD.h"

namespace AudioEngineFMOD
{

IAudioSystemCtrl_FMOD::IAudioSystemCtrl_FMOD(const AZStd::string &name, AudioControls::CID id, AudioControls::TImplControlType type)
    : AudioControls::IAudioSystemControl(name, id, type)
{

}

}