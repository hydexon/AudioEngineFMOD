
#include <AudioEngineFMOD/AudioEngineFMODTypeIds.h>
#include <AudioEngineFMODModuleInterface.h>
#include "AudioEngineFMODSystemComponent.h"

namespace AudioEngineFMOD
{
    class AudioEngineFMODModule
        : public AudioEngineFMODModuleInterface
    {
    public:
        AZ_RTTI(AudioEngineFMODModule, AudioEngineFMODModuleTypeId, AudioEngineFMODModuleInterface);
        AZ_CLASS_ALLOCATOR(AudioEngineFMODModule, AZ::SystemAllocator);
    };
}// namespace AudioEngineFMOD

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), AudioEngineFMOD::AudioEngineFMODModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_AudioEngineFMOD, AudioEngineFMOD::AudioEngineFMODModule)
#endif
