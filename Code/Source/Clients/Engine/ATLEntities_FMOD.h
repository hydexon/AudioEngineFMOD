#pragma once

#include <ATLEntityData.h>
#include "Common_FMOD.h"

namespace AudioEngineFMOD
{
    struct SATLAudioObjectData_FMOD : public Audio::IATLAudioObjectData
    {
        InstancesMultiMap m_activeInstances;
    };


    struct SATLTriggerImplData_FMOD : public Audio::IATLTriggerImplData
    {
        AZStd::string eventPath;
        bool preloadSampleData;
    };

    struct SATLListenerData_FMOD : public Audio::IATLListenerData
    {
        int listenerIndex = 0;
        AZ::Vector3 velocity = AZ::Vector3::CreateOne();
        float weight = 1.0f;
    };

}