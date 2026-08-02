
#include "AudioEngineFMODSystemComponent.h"

#include <AudioEngineFMOD/AudioEngineFMODTypeIds.h>

#include <AzCore/Serialization/SerializeContext.h>

#include <AudioAllocators.h>
#include <IAudioSystem.h>

#include "Engine/AudioSystemImpl_FMOD.h"
#include "Engine/ConfigFMOD.h"

#include <AzCore/Settings/SettingsRegistryMergeUtils.h>
#include <AzFramework/Platform/PlatformDefaults.h>

namespace AudioEngineFMOD
{

    void AudioEngineFMODSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<AudioEngineFMODSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }

        FMODLocaleConfig::Reflect(context);
    }

    void AudioEngineFMODSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("AudioEngineService"));
    }

    void AudioEngineFMODSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("AudioEngineService"));
    }

    void AudioEngineFMODSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("AudioSystemService"));
    }

    void AudioEngineFMODSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("AudioSystemService"));
    }

    void AudioEngineFMODSystemComponent::Init()
    {
    }

    bool AudioEngineFMODSystemComponent::Initialize()
    {
        AZ::SettingsRegistryInterface::FixedValueString assetPlatform = AzFramework::OSPlatformToDefaultAssetPlatform(
            AZ_TRAIT_OS_PLATFORM_CODENAME);
        if (assetPlatform.empty())
        {
            if (const auto settingsRegistry = AZ::SettingsRegistry::Get(); settingsRegistry != nullptr)
            {
                AZ::SettingsRegistryMergeUtils::PlatformGet(*settingsRegistry, assetPlatform,
                    AZ::SettingsRegistryMergeUtils::BootstrapSettingsRootKey, "assets");
            }
        }

        AZ_Info("FMODAudioSystem", "AudioEngineFMOD AssetPlatform: %s", assetPlatform.c_str());
        m_engineFMOD = AZStd::make_unique<AudioSystemImpl_FMOD>(assetPlatform.c_str());
        if(m_engineFMOD)
        {
            AZ_Info("FMODAudioSystem", "AudioEngineFMOD Initialized!");
            Audio::SystemRequest::Initialize initReq;
            AZ::Interface<Audio::IAudioSystem>::Get()->PushRequestBlocking(AZStd::move(initReq));
        }
        return true;
    }

    void AudioEngineFMODSystemComponent::Release()
    {
        m_engineFMOD.reset();
    }

    void AudioEngineFMODSystemComponent::Activate()
    {
        Audio::Gem::EngineRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void AudioEngineFMODSystemComponent::Deactivate()
    {
        Audio::Gem::EngineRequestBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
    }

    void AudioEngineFMODSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace AudioEngineFMOD
