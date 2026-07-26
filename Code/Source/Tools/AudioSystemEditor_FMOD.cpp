#include "AudioSystemEditor_FMOD.h"
#include "AudioSystemCtrl_FMOD.h"
#include "../Clients/Engine/ConfigFMOD.h"

#include <AzCore/Utils/Utils.h>
#include <IAudioSystem.h>
#include <AzCore/std/smart_ptr/make_shared.h>

//NOLINTBEGIN
void InitFMODResources()
{
    Q_INIT_RESOURCE(EditorFMODIcons);
}
//NOLINTEND

using namespace AudioControls;

namespace AudioEngineFMOD
{

CAudioSystemEditor_FMOD::CAudioSystemEditor_FMOD()
{
    InitFMODResources();
}

void CAudioSystemEditor_FMOD::Reload()
{
    // set all the controls as placeholder as we don't know if
    // any of them have been removed but still have connections to them
    for (const auto& idControlPair : m_controls)
    {
        TControlPtr control = idControlPair.second;
        if (control)
        {
            control->SetPlaceholder(true);
        }
    }

    m_loader.Load(this);
}

AudioControls::IAudioSystemControl *CAudioSystemEditor_FMOD::CreateControl(const AudioControls::SControlDef &controlDefinition)
{
    AZStd::string fullName = controlDefinition.m_name;
    IAudioSystemControl* parent = controlDefinition.m_parentControl;
    if (parent)
    {
        AZ::StringFunc::Path::Join(controlDefinition.m_parentControl->GetName().c_str(), fullName.c_str(), fullName);
    }

    if (!controlDefinition.m_path.empty())
    {
        AZ::StringFunc::Path::Join(controlDefinition.m_path.c_str(), fullName.c_str(), fullName);
    }

    CID id = Audio::AudioStringToID<CID>(fullName.c_str());

    AudioControls::IAudioSystemControl* control = GetControl(id);
    if (control)
    {
        if (control->IsPlaceholder())
        {
            control->SetPlaceholder(false);
            if (parent && parent->IsPlaceholder())
            {
                parent->SetPlaceholder(false);
            }
        }
        return control;
    }
    else
    {
        TControlPtr newControl = AZStd::make_shared<IAudioSystemCtrl_FMOD>(controlDefinition.m_name, id, controlDefinition.m_type);
        if (!parent)
        {
            parent = &m_rootControl;
        }

        parent->AddChild(newControl.get());
        newControl->SetParent(parent);
        newControl->SetLocalized(controlDefinition.m_isLocalized);
        m_controls[id] = newControl;
        return newControl.get();
    }
}

AudioControls::IAudioSystemControl *CAudioSystemEditor_FMOD::GetRoot()
{
    return &m_rootControl;
}

AudioControls::IAudioSystemControl *CAudioSystemEditor_FMOD::GetControl(AudioControls::CID id) const
{
    if (id != ACE_INVALID_CID)
    {
        auto it = m_controls.find(id);
        if (it != m_controls.end())
        {
            return it->second.get();
        }
    }
    return nullptr;
}

AudioControls::EACEControlType CAudioSystemEditor_FMOD::ImplTypeToATLType(AudioControls::TImplControlType type) const
{
    switch(type)
    {
    case eFMOD_EVENT:
        return AudioControls::eACET_TRIGGER;
    case eFMOD_PARAMETER:
        return AudioControls::eACET_RTPC;
    case eFMOD_LABELEDPARAM:
        return AudioControls::eACET_SWITCH_STATE;
    case eFMOD_AUXBUS:
        return AudioControls::eACET_ENVIRONMENT;
    case eFMOD_SOUNDBANK:
        return AudioControls::eACET_PRELOAD;
    case eFMOD_SNAPSHOT:
        return AudioControls::eACET_ENVIRONMENT;
    }

    return AudioControls::eACET_NUM_TYPES;
}

AudioControls::TImplControlTypeMask CAudioSystemEditor_FMOD::GetCompatibleTypes(AudioControls::EACEControlType atlControlType) const
{
    switch(atlControlType)
    {
    case AudioControls::eACET_TRIGGER:
        return eFMOD_EVENT;
    case AudioControls::eACET_RTPC:
        return eFMOD_PARAMETER;
    case AudioControls::eACET_SWITCH:
        return eFMOD_LABELEDPARAM;
    case AudioControls::eACET_SWITCH_STATE:
        return eFMOD_SNAPSHOT;
    case AudioControls::eACET_ENVIRONMENT:
        return eFMOD_AUXBUS;
    case AudioControls::eACET_PRELOAD:
        return eFMOD_SOUNDBANK;
    default:
        break;
    }
    return AudioControls::AUDIO_IMPL_INVALID_TYPE;
}

AudioControls::TConnectionPtr CAudioSystemEditor_FMOD::CreateConnectionToControl(AudioControls::EACEControlType atlControlType, AudioControls::IAudioSystemControl *middlewareControl)
{
    return nullptr;
}

AudioControls::TConnectionPtr CAudioSystemEditor_FMOD::CreateConnectionFromXMLNode(AZ::rapidxml::xml_node<char> *node, AudioControls::EACEControlType atlControlType)
{
    return nullptr;
}

AZ::rapidxml::xml_node<char> *CAudioSystemEditor_FMOD::CreateXMLNodeFromConnection(const AudioControls::TConnectionPtr connection, const AudioControls::EACEControlType atlControlType)
{
    return nullptr;
}

const AZStd::string_view CAudioSystemEditor_FMOD::GetTypeIcon(AudioControls::TImplControlType type) const
{
    switch(type)
    {
    case eFMOD_EVENT:
        return ":/Editor/EditorFMODIcons/events.png";
    case eFMOD_PARAMETER:
        return ":/Editor/EditorFMODIcons/switch.png";
    case eFMOD_AUXBUS:
        return ":/Editor/EditorFMODIcons/bus.png";
    case eFMOD_SOUNDBANK:
        return ":/Editor/EditorFMODIcons/soundbank.png";
    case eFMOD_SNAPSHOT:
        return ":/Editor/EditorFMODIcons/snapshot.png";
    default:
        return ":/Editor/EditorFMODIcons/invalid.png";
    }
}

const AZStd::string_view CAudioSystemEditor_FMOD::GetTypeIconSelected(AudioControls::TImplControlType type) const
{
    return GetTypeIcon(type);
}

AZStd::string CAudioSystemEditor_FMOD::GetName() const
{
    return "FMOD Studio";
}

AZ::IO::FixedMaxPath CAudioSystemEditor_FMOD::GetDataPath() const
{
    auto projectPath = AZ::IO::FixedMaxPath( AZ::Utils::GetProjectPath() );
    return (projectPath / "Assets" / "Audio" / "FMOD"  );
}

void CAudioSystemEditor_FMOD::DataSaved()
{

}


}