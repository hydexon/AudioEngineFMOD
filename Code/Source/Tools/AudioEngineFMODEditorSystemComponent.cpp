
#include <AzCore/Serialization/SerializeContext.h>
#include "AudioEngineFMODEditorSystemComponent.h"

#include <AudioEngineFMOD/AudioEngineFMODTypeIds.h>

#include <AudioAllocators.h>
#include <IAudioSystemEditor.h>
#include "AudioSystemEditor_FMOD.h"

namespace AudioEngineFMOD
{

    void AudioEngineFMODEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<AudioEngineFMODEditorSystemComponent, AudioEngineFMODSystemComponent>()
                ->Version(0);

            if(AZ::EditContext* ec = serializeContext->GetEditContext())
            {
                ec->Class<AudioEngineFMODEditorSystemComponent>("Audio Engine FMOD Studio Gem", "FMOD Implementation of the Audio Engine interfaces")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                            ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }
    }

    AudioEngineFMODEditorSystemComponent::AudioEngineFMODEditorSystemComponent() = default;

    AudioEngineFMODEditorSystemComponent::~AudioEngineFMODEditorSystemComponent() = default;

    void AudioEngineFMODEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
    }

    void AudioEngineFMODEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
    }

    void AudioEngineFMODEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void AudioEngineFMODEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void AudioEngineFMODEditorSystemComponent::InitializeEditorImplPlugin()
    {
        m_editorImplPlugin.reset(new CAudioSystemEditor_FMOD());
    }

    void AudioEngineFMODEditorSystemComponent::ReleaseEditorImplPlugin()
    {
        m_editorImplPlugin.release();
    }

    AudioControls::IAudioSystemEditor* AudioEngineFMODEditorSystemComponent::GetEditorImplPlugin()
    {
        return m_editorImplPlugin.get();
    }

    void AudioEngineFMODEditorSystemComponent::Activate()
    {
        AudioEngineFMODSystemComponent::Activate();
        AudioControlsEditor::EditorImplPluginEventBus::Handler::BusConnect();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void AudioEngineFMODEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        AudioControlsEditor::EditorImplPluginEventBus::Handler::BusDisconnect();
        AudioEngineFMODSystemComponent::Deactivate();
    }

} // namespace AudioEngineFMOD
