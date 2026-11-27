#include "AudioSystemImplCVars.h"
#include <fmod_common.h>

namespace AudioEngineFMOD::CVars
{
    AZ_CVAR(int,
            s_FMODStudio_MaxChannels,
            512,
            nullptr,
            AZ::ConsoleFunctorFlags::Null,
            "The maximum number of Channels, including both virtual and real, to be used in FMOD");

    AZ_CVAR(bool,
            s_FMODStudio_PauseAudioOnFocusLost,
            true,
            nullptr,
            AZ::ConsoleFunctorFlags::Null,
            "FMOD Pauses Audio when the game loses focus."
            );

    AZ_CVAR(float,
            s_FMODStudio_DSPLpfMinCutoffFreq,
            10.0f,
            nullptr,
            AZ::ConsoleFunctorFlags::Null,
            "Sets the minimum LPF cutoff frequency during full audio object occlusion.");
}