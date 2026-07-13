
#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Clients/AudioEngineFMODSystemComponent.h>

namespace AudioEngineFMOD
{
    /// System component for AudioEngineFMOD editor
    class AudioEngineFMODEditorSystemComponent
        : public AudioEngineFMODSystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = AudioEngineFMODSystemComponent;
    public:
        AZ_COMPONENT_DECL(AudioEngineFMODEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        AudioEngineFMODEditorSystemComponent();
        ~AudioEngineFMODEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;
    };
} // namespace AudioEngineFMOD
