
#include "AudioEngineFMODModuleInterface.h"
#include <AzCore/Memory/Memory.h>

#include <AudioEngineFMOD/AudioEngineFMODTypeIds.h>

#include <Clients/AudioEngineFMODSystemComponent.h>

namespace AudioEngineFMOD
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(AudioEngineFMODModuleInterface,
        "AudioEngineFMODModuleInterface", AudioEngineFMODModuleInterfaceTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(AudioEngineFMODModuleInterface, AZ::Module);
    AZ_CLASS_ALLOCATOR_IMPL(AudioEngineFMODModuleInterface, AZ::SystemAllocator);

    AudioEngineFMODModuleInterface::AudioEngineFMODModuleInterface()
    {
        // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
        // Add ALL components descriptors associated with this gem to m_descriptors.
        // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
        // This happens through the [MyComponent]::Reflect() function.
        m_descriptors.insert(m_descriptors.end(), {
            AudioEngineFMODSystemComponent::CreateDescriptor(),
            });
    }

    AZ::ComponentTypeList AudioEngineFMODModuleInterface::GetRequiredSystemComponents() const
    {
        return AZ::ComponentTypeList{
            azrtti_typeid<AudioEngineFMODSystemComponent>(),
        };
    }
} // namespace AudioEngineFMOD
