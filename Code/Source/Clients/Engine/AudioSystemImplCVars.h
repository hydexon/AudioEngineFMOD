#pragma once

#include <AzCore/Console/IConsole.h>

namespace AudioEngineFMOD::CVars
{
    AZ_CVAR_EXTERNED(int, s_FMODStudio_MaxChannels);
    AZ_CVAR_EXTERNED(bool, s_FMODStudio_PauseAudioOnFocusLost);
    AZ_CVAR_EXTERNED(float, s_FMODStudio_DSPLpfMinCutoffFreq);
}