
#include <AzCore/Serialization/SerializeContext.h>
#include "AudioEngineFMODEditorSystemComponent.h"

#include <AudioEngineFMOD/AudioEngineFMODTypeIds.h>

namespace AudioEngineFMOD
{
    AZ_COMPONENT_IMPL(AudioEngineFMODEditorSystemComponent, "AudioEngineFMODEditorSystemComponent",
        AudioEngineFMODEditorSystemComponentTypeId, BaseSystemComponent);

    void AudioEngineFMODEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<AudioEngineFMODEditorSystemComponent, AudioEngineFMODSystemComponent>()
                ->Version(0);
        }
    }

    AudioEngineFMODEditorSystemComponent::AudioEngineFMODEditorSystemComponent() = default;

    AudioEngineFMODEditorSystemComponent::~AudioEngineFMODEditorSystemComponent() = default;

    void AudioEngineFMODEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("AudioEngineFMODEditorService"));
    }

    void AudioEngineFMODEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("AudioEngineFMODEditorService"));
    }

    void AudioEngineFMODEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void AudioEngineFMODEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void AudioEngineFMODEditorSystemComponent::Activate()
    {
        AudioEngineFMODSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void AudioEngineFMODEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        AudioEngineFMODSystemComponent::Deactivate();
    }

} // namespace AudioEngineFMOD
