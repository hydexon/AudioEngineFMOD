#pragma once

#include <AzCore/Console/IConsole.h>

namespace AudioEngineFMOD::CVars
{
    AZ_CVAR_EXTERNED(int, s_FMODStudio_MaxChannels);
    AZ_CVAR_EXTERNED(bool,s_FMODStudio_EnableProfiling);

    AZ_CVAR_EXTERNED(uint32_t, s_FMODStudio_CommandQueueSize);
    AZ_CVAR_EXTERNED(int, s_FMODStudio_StudioUpdatePeriod);
    AZ_CVAR_EXTERNED(int, s_FMODStudio_IdleSampleDataPoolSize);
    AZ_CVAR_EXTERNED(uint32_t, s_FMODStudio_StreamingScheduleDelay);

    AZ_CVAR_EXTERNED(uint16_t, s_FMODCore_ProfilePort);
    AZ_CVAR_EXTERNED(bool, s_FMODStudio_PauseAudioOnFocusLost);
}