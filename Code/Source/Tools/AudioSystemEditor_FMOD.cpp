#include "AudioSystemEditor_FMOD.h"
#include "AudioSystemCtrl_FMOD.h"

#include <AzCore/Utils/Utils.h>
#include <IAudioSystem.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/Console/IConsole.h>

#include "../Clients/Engine/Common_FMOD.h"
#include "AudioConnections.h"

#include "ConnectionWidgets/LoadSampleDataForm.h"
#include "ConnectionWidgets/EventPropertiesForm.h"

void InitFMODResources()
{
    Q_INIT_RESOURCE(EditorFMODIcons);
}

using namespace AudioControls;

namespace AudioEngineFMOD
{

TImplControlType TagToType(const AZStd::string_view tag)
{
#if 0
    if(tag == XMLTags::FMODEventTag)
    {
        return eFMOD_EVENT;
    }
    else if(tag == XMLTags::FMODParameterTag)
    {
        return eFMOD_PARAMETER;
    }
    else if(tag == XMLTags::FMODStudioBankTag)
    {
        return eFMOD_SOUNDBANK;
    }
#endif
    return eFMOD_INVALID;
}

const AZStd::string_view TypeToTag(const TImplControlType type)
{
#if 0
    switch(type)
    {
    case eFMOD_EVENT:
        return XMLTags::FMODEventTag;
    case eFMOD_PARAMETER:
        return XMLTags::FMODParameterTag;
    case eFMOD_SOUNDBANK:
        return XMLTags::FMODStudioBankTag;
    default:
        break;
    }
#endif
    return "";
}


CAudioSystemEditor_FMOD::CAudioSystemEditor_FMOD()
    : QObject()
{
    InitFMODResources();

    m_localizedParentControl.SetParent(&m_rootControl);
    m_rootControl.AddChild(&m_localizedParentControl);

    m_loader.Load(this);
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
    return &m_rootControl;
}

AudioControls::IAudioSystemControl *CAudioSystemEditor_FMOD::GetControl(AudioControls::CID id) const
{
    return nullptr;
}

AudioControls::EACEControlType CAudioSystemEditor_FMOD::ImplTypeToATLType(AudioControls::TImplControlType type) const
{
    return AudioControls::eACET_NUM_TYPES;
}

AudioControls::TImplControlTypeMask CAudioSystemEditor_FMOD::GetCompatibleTypes(AudioControls::EACEControlType atlControlType) const
{
    return AudioControls::AUDIO_IMPL_INVALID_TYPE;
}

AudioControls::TConnectionPtr CAudioSystemEditor_FMOD::CreateConnectionToControl(AudioControls::EACEControlType atlControlType, AudioControls::IAudioSystemControl *middlewareControl)
{
    return nullptr;
}

AudioControls::TConnectionPtr CAudioSystemEditor_FMOD::CreateConnectionFromXMLNode(AZ::rapidxml::xml_node<char> *node, [[maybe_unused]]AudioControls::EACEControlType atlControlType)
{
    using namespace Audio;
    if(!node)
    {
        return nullptr;
    }
    return nullptr;
}

AZ::rapidxml::xml_node<char> *CAudioSystemEditor_FMOD::CreateXMLNodeFromConnection(const AudioControls::TConnectionPtr connection, const AudioControls::EACEControlType atlControlType)
{
    return nullptr;
}

const AZStd::string_view CAudioSystemEditor_FMOD::GetTypeIcon(AudioControls::TImplControlType type) const
{
    return ":/Editor/EditorFMODIcons/invalid.png";
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
    return (projectPath / "Assets" / "Audio" / "FMOD" / "FMOD_Project" );
}

void CAudioSystemEditor_FMOD::DataSaved()
{

}

QWidget *CAudioSystemEditor_FMOD::CreateConnectionPropertiesWidget(const AudioControls::TConnectionPtr connection, [[maybe_unused]] AudioControls::EACEControlType atlControlType)
{
    if(!connection)
        return nullptr;

    IAudioSystemControl* control = GetControl(connection->GetID());
    if(!control)
        return nullptr;

    return nullptr;
}

IAudioSystemControl *CAudioSystemEditor_FMOD::GetControlByName(AZStd::string name, bool isLocalized, AudioControls::IAudioSystemControl *parent) const
{
    if (parent) //This is definitively used for the Switch Groups.
    {
        AZ::StringFunc::Path::Join(parent->GetName().c_str(), name.c_str(), name);
    }

    if(isLocalized)
    {
        AZ::StringFunc::Path::Join(m_currentLanguageName.c_str(), name.c_str(), name);
    }

    return GetControl(Audio::AudioStringToID<CID>(name.data()));
}


}