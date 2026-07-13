
#include "AudioEngineFMODSystemComponent.h"

#include <AudioEngineFMOD/AudioEngineFMODTypeIds.h>

#include <AzCore/Serialization/SerializeContext.h>

namespace AudioEngineFMOD
{
    AZ_COMPONENT_IMPL(AudioEngineFMODSystemComponent, "AudioEngineFMODSystemComponent",
        AudioEngineFMODSystemComponentTypeId);

    void AudioEngineFMODSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<AudioEngineFMODSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void AudioEngineFMODSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("AudioEngineFMODService"));
    }

    void AudioEngineFMODSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("AudioEngineFMODService"));
    }

    void AudioEngineFMODSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void AudioEngineFMODSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    AudioEngineFMODSystemComponent::AudioEngineFMODSystemComponent()
    {
        if (AudioEngineFMODInterface::Get() == nullptr)
        {
            AudioEngineFMODInterface::Register(this);
        }
    }

    AudioEngineFMODSystemComponent::~AudioEngineFMODSystemComponent()
    {
        if (AudioEngineFMODInterface::Get() == this)
        {
            AudioEngineFMODInterface::Unregister(this);
        }
    }

    void AudioEngineFMODSystemComponent::Init()
    {
    }

    void AudioEngineFMODSystemComponent::Activate()
    {
        AudioEngineFMODRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void AudioEngineFMODSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        AudioEngineFMODRequestBus::Handler::BusDisconnect();
    }

    void AudioEngineFMODSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace AudioEngineFMOD
