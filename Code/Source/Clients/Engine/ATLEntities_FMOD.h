#pragma once

#include <ATLEntityData.h>
#include <fmod_studio.hpp>

namespace AudioEngineFMOD
{
    struct SATLAudioObjectData_FMOD : public Audio::IATLAudioObjectData
    {
        FMOD_3D_ATTRIBUTES m_attributes;
        FMOD::Studio::EventInstance* m_instance;
    };


    struct SATLTriggerImplData_FMOD : public Audio::IATLTriggerImplData
    {
        Audio::EAudioEventState audioEventState;
        AZStd::string eventPath;
        bool preloadSampleData;
    };

}