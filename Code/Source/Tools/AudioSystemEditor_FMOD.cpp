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
    , m_banksControl("Banks", ACE_INVALID_CID, eFMOD_GROUPCTRL)
    , m_eventsControl("Events", ACE_INVALID_CID, eFMOD_GROUPCTRL)
    , m_snapshotsCtrl("Snapshots", ACE_INVALID_CID, eFMOD_GROUPCTRL)
    , m_globalParamsCtrl("Global Parameters", ACE_INVALID_CID, eFMOD_GROUPCTRL)
    , m_returnsControl("Returns", ACE_INVALID_CID, eFMOD_GROUPCTRL)
{
    InitFMODResources();

    m_localizedParentControl.SetParent(&m_rootControl);
    m_rootControl.AddChild(&m_localizedParentControl);

    m_banksControl.SetParent(&m_rootControl);
    m_rootControl.AddChild(&m_banksControl);

    m_eventsControl.SetParent(&m_rootControl);
    m_rootControl.AddChild(&m_eventsControl);

    m_snapshotsCtrl.SetParent(&m_rootControl);
    m_rootControl.AddChild(&m_snapshotsCtrl);

    m_globalParamsCtrl.SetParent(&m_rootControl);
    m_rootControl.AddChild(&m_globalParamsCtrl);

    m_returnsControl.SetParent(&m_rootControl);
    m_rootControl.AddChild(&m_returnsControl);

    m_loader.Load(this);
}

void CAudioSystemEditor_FMOD::Reload()
{
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
    if (AudioControls::IAudioSystemControl* control = GetControl(id))
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
        if (auto it = m_controls.find(id); it != m_controls.end())
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
    case eFMOD_SNAPSHOT:
        return AudioControls::eACET_TRIGGER;
    case eFMOD_EVENT_PARAMETER:
    case eFMOD_SNAPSHOT_PARAMETER:
    case eFMOD_GLOBAL_PARAMETER:
        return AudioControls::eACET_RTPC;
    case eFMOD_SOUNDBANK:
        return AudioControls::eACET_PRELOAD;
    case eFMOD_RETURN:
        return AudioControls::eACET_ENVIRONMENT;
    default:
        return AudioControls::eACET_NUM_TYPES;
    }
}

AudioControls::TImplControlTypeMask CAudioSystemEditor_FMOD::GetCompatibleTypes(AudioControls::EACEControlType atlControlType) const
{
    switch(atlControlType)
    {
    case AudioControls::eACET_PRELOAD:
        return eFMOD_SOUNDBANK;
    case AudioControls::eACET_TRIGGER:
        return eFMOD_EVENT | eFMOD_SNAPSHOT;
    case AudioControls::eACET_RTPC:
        return eFMOD_EVENT_PARAMETER | eFMOD_SNAPSHOT_PARAMETER | eFMOD_GLOBAL_PARAMETER;
    case AudioControls::eACET_SWITCH:
    case AudioControls::eACET_SWITCH_STATE:
        return eFMOD_EVENT_PARAMETER | eFMOD_SNAPSHOT_PARAMETER | eFMOD_GLOBAL_PARAMETER;
    case AudioControls::eACET_ENVIRONMENT:
        return eFMOD_RETURN;
    default:
        return AudioControls::AUDIO_IMPL_INVALID_TYPE;
    }
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
    switch (type)
    {
    case eFMOD_SOUNDBANK:
        return ":/AEFMODIcons/bank.png";
    case eFMOD_GROUPCTRL:
        return ":/AEFMODIcons/folder-horizontal.png";
    case eFMOD_EVENT:
        return ":/AEFMODIcons/event.png";
    case eFMOD_SNAPSHOT:
        return ":/AEFMODIcons/snapshot.png";
    case eFMOD_EVENT_PARAMETER:
    case eFMOD_SNAPSHOT_PARAMETER:
    case eFMOD_GLOBAL_PARAMETER:
        return ":/AEFMODIcons/tag_icon.png";
    case eFMOD_FOLDER:
        return ":/AEFMODIcons/folder_closed.png";
    case eFMOD_RETURN:
        return ":/AEFMODIcons/return.png";
    default:
        return ":/AEFMODIcons/folder-horizontal.png";
    }
}

const AZStd::string_view CAudioSystemEditor_FMOD::GetTypeIconSelected(AudioControls::TImplControlType type) const
{
    switch (type)
    {
    case eFMOD_GROUPCTRL:
        return ":/AEFMODIcons/folder-horizontal-open.png";
    case eFMOD_FOLDER:
        return ":/AEFMODIcons/folder_open.png";
    default:
        return GetTypeIcon(type);
    }
}

AZStd::string CAudioSystemEditor_FMOD::GetName() const
{
    return "FMOD Studio";
}

AZ::IO::FixedMaxPath CAudioSystemEditor_FMOD::GetDataPath() const
{
    auto projectPath = AZ::IO::FixedMaxPath( AZ::Utils::GetProjectPath() );
    return (projectPath / "Assets" / "Audio" / "FMOD" );
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