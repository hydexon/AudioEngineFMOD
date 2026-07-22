#include "AudioSystemEditor_FMOD.h"

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
    return AudioControls::eACET_TRIGGER;

}

AudioControls::TImplControlTypeMask CAudioSystemEditor_FMOD::GetCompatibleTypes(AudioControls::EACEControlType atlControlType) const
{
    return 0;
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
    return "";
}

const AZStd::string_view CAudioSystemEditor_FMOD::GetTypeIconSelected(AudioControls::TImplControlType type) const
{
    return "";
}

AZStd::string CAudioSystemEditor_FMOD::GetName() const
{
    return "FMOD Studio";
}

AZ::IO::FixedMaxPath CAudioSystemEditor_FMOD::GetDataPath() const
{
    return "";
}

void CAudioSystemEditor_FMOD::DataSaved()
{

}


}