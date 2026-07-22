
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AudioEngineFMOD/AudioEngineFMODBus.h>
#include <IAudioSystem.h>
#include <IAudioSystemImplementation.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace AudioEngineFMOD
{
    class AudioEngineFMODSystemComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , protected Audio::Gem::EngineRequestBus::Handler
    {
    public:
        AZ_COMPONENT(AudioEngineFMODSystemComponent, "{B0C9C624-75F4-45BC-A03E-A0663F475493}", AZ::Component);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // Audio::Gem::EngineRequestBus interface implementation
        bool Initialize() override;
        void Release();
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZTickBus interface implementation
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        ////////////////////////////////////////////////////////////////////////
    private:
        AZStd::unique_ptr<Audio::AudioSystemImplementation> m_engineFMOD;
    };

} // namespace AudioEngineFMOD
