#include "AudioSystemImplCVars.h"


namespace AudioEngineFMOD::CVars
{
    AZ_CVAR(int,
            s_FMODStudio_MaxChannels,
            512,
            nullptr,
            AZ::ConsoleFunctorFlags::Null,
            "The maximum number of Channels, including both virtual and real, to be used in FMOD");

    AZ_CVAR(bool,
            s_FMODStudio_EnableProfiling,
            false,
            nullptr,
            AZ::ConsoleFunctorFlags::Null,
            "Enable/Disable FMOD Profiling, see s_FMODCore_ProfilePort to find the profiling port");

    AZ_CVAR(uint32_t,
            s_FMODStudio_CommandQueueSize,
            32768,
            nullptr,
            AZ::ConsoleFunctorFlags::Null,
            "Command queue size for studio async processing (in Bytes)");

    AZ_CVAR(int,
            s_FMODStudio_StudioUpdatePeriod,
            20,
            nullptr,
            AZ::ConsoleFunctorFlags::Null,
            "Update period of Studio when in async mode, in milliseconds. Will be quantized to the nearest multiple of mixer duration. (in Milliseconds)");

    AZ_CVAR(int,
            s_FMODStudio_IdleSampleDataPoolSize,
            262144,
            nullptr,
            AZ::ConsoleFunctorFlags::Null,
            "Size in bytes of sample data to retain in memory when no longer used, to avoid repeated disk I/O. Use -1 to disable.");

    AZ_CVAR(uint32_t,
            s_FMODStudio_StreamingScheduleDelay,
            8192,
            nullptr,
            AZ::ConsoleFunctorFlags::Null,
            "Specify the schedule delay for streams, in samples. Lower values can reduce latency when scheduling events containing streams but may cause scheduling issues if too small.");

    AZ_CVAR(uint16_t,
            s_FMODCore_ProfilePort,
            9264,
            nullptr,
            AZ::ConsoleFunctorFlags::Null,
            "Used when FMOD Profiler is Enabled and the s_FMODStudio_EnableProfiling is enabled.");
}