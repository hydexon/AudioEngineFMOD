
#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Clients/AudioEngineFMODSystemComponent.h>
#include <IAudioSystemEditor.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace AudioEngineFMOD
{
    /// System component for AudioEngineFMOD editor
    class AudioEngineFMODEditorSystemComponent
        : public AudioEngineFMODSystemComponent
        , protected AudioControlsEditor::EditorImplPluginEventBus::Handler
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = AudioEngineFMODSystemComponent;
    public:
        AZ_COMPONENT(AudioEngineFMODEditorSystemComponent, "{41C71BBB-4AB5-496F-9F40-41D4936C2375}", AZ::Component);

        static void Reflect(AZ::ReflectContext* context);

        AudioEngineFMODEditorSystemComponent();
        ~AudioEngineFMODEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AudioControlsEditor::EditorImplPluginEventBus:
        void InitializeEditorImplPlugin() override;
        void ReleaseEditorImplPlugin() override;
        AudioControls::IAudioSystemEditor* GetEditorImplPlugin() override;

        // AZ::Component
        void Activate() override;
        void Deactivate() override;

        //Other:
        AZStd::unique_ptr<AudioControls::IAudioSystemEditor> m_editorImplPlugin;

    };
} // namespace AudioEngineFMOD
