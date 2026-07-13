
#include <AudioEngineFMOD/AudioEngineFMODTypeIds.h>
#include <AudioEngineFMODModuleInterface.h>
#include "AudioEngineFMODEditorSystemComponent.h"

namespace AudioEngineFMOD
{
    class AudioEngineFMODEditorModule
        : public AudioEngineFMODModuleInterface
    {
    public:
        AZ_RTTI(AudioEngineFMODEditorModule, AudioEngineFMODEditorModuleTypeId, AudioEngineFMODModuleInterface);
        AZ_CLASS_ALLOCATOR(AudioEngineFMODEditorModule, AZ::SystemAllocator);

        AudioEngineFMODEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                AudioEngineFMODEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<AudioEngineFMODEditorSystemComponent>(),
            };
        }
    };
}// namespace AudioEngineFMOD

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), AudioEngineFMOD::AudioEngineFMODEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_AudioEngineFMOD_Editor, AudioEngineFMOD::AudioEngineFMODEditorModule)
#endif
