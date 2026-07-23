#include "AudioSystemEditor_FMOD.h"
#include "AudioSystemCtrl_FMOD.h"
#include <AzCore/Utils/Utils.h>
#include "../Clients/Engine/ConfigFMOD.h"

namespace AudioEngineFMOD
{

CAudioSystemEditor_FMOD::CAudioSystemEditor_FMOD()
{

}

void CAudioSystemEditor_FMOD::Reload()
{

}

AudioControls::IAudioSystemControl *CAudioSystemEditor_FMOD::CreateControl(const AudioControls::SControlDef &controlDefinition)
{
    return nullptr;
}

AudioControls::IAudioSystemControl *CAudioSystemEditor_FMOD::GetRoot()
{
    return nullptr;
}

AudioControls::IAudioSystemControl *CAudioSystemEditor_FMOD::GetControl(AudioControls::CID id) const
{
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
    return (projectPath / "Assets/Audio/FMOD" );
}

void CAudioSystemEditor_FMOD::DataSaved()
{

}


}